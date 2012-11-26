//#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "fev_listener.h"
#include "fev_buff.h"
#include "fev_timer.h"
#include "tu_inc.h"

#include "http_handlers.h"

static char fake_response[] =
"HTTP/1.1 200 OK\r\n"
"Date: Tue, 13 Nov 2012 13:21:30 GMT\r\n"
"Server: Http mock\r\n"
"Expires: Wed, 14 Nov 2012 13:21:30 GMT\r\n"
"Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
"Accept-Ranges: bytes\r\n"
"Content-Length: 86\r\n"
"Connection: Keep-Alive\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<html>\r\n"
"<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
"</html>\r\n";

/**
 * design mode:
 *     client_mgr
 *    /     |    \
 *client client client
 *   |      |     |
 * timer  timer  timer
 */

struct client;
struct timer_mgr;
struct client_mgr;

typedef struct timer_node {
    struct client*     cli;
    struct timer_node* prev;
    struct timer_node* next;
    struct timer_mgr*  owner;
    int                timeout; // unit [ms]
} timer_node;

typedef struct client {
    int         fd;
    int         offset;
    int         request_complete;
    int         response_complete;
    my_time     last_active;
    fev_buff*   evbuff;
    timer_node* tnidx;
    struct client_mgr* owner;
} client;

typedef struct timer_mgr {
    timer_node* head;
    timer_node* tail;
    int         count;
} timer_mgr;

typedef struct client_mgr {
    timer_mgr  tm_main;
    timer_mgr  tm_minor;
    timer_mgr* current;
    timer_mgr* backup;
    int max_open_files;
    int current_conn;
} client_mgr;

static fev_state* fev = NULL;
static client_mgr* cli_mgr = NULL;

static
timer_node* timer_node_create(client* cli, int timeout)
{
    timer_node* tnode = malloc(sizeof(timer_node));
    memset(tnode, 0, sizeof(*tnode));
    cli->tnidx = tnode;
    tnode->cli = cli;
    tnode->timeout = timeout;
    tnode->prev = tnode->next = NULL;

    return tnode;
}

static
void timer_node_delete(timer_mgr* mgr, timer_node* node)
{
    if ( !node )
        return;
    if ( !node->owner )
        goto RELEASE;

    //assert(node->owner == mgr);
    if ( !node->prev ) { // node at head
        mgr->head = node->next;
        if ( mgr->head ) mgr->head->prev = NULL;
    } else if ( !node->next ) { // node at tail
        mgr->tail = node->prev;
        if ( mgr->tail ) mgr->tail->next = NULL;
    } else { // node at middle
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    node->owner = NULL;
    mgr->count--;
    if ( !mgr->count ) {
        mgr->head = mgr->tail = NULL;
    }

RELEASE:
    free(node);
}

static
void timer_node_push(timer_mgr* mgr, timer_node* node)
{
    //assert( !node->owner );
    if ( mgr->head == mgr->tail && mgr->head == NULL ) {
        mgr->head = mgr->tail = node;
    } else {
        node->prev = mgr->tail;
        mgr->tail->next = node;
        mgr->tail = node;
    }

    node->owner = mgr;
    mgr->count++;
}

static
timer_node* timer_node_pop(timer_mgr* mgr)
{
    if ( !mgr->head ) {
        return NULL;
    } else {
        timer_node* node = mgr->head;
        mgr->head = mgr->head->next;
        if ( !mgr->head ) {
            mgr->tail = mgr->head;
        } else {
            mgr->head->prev = NULL;
        }

        node->prev = node->next = NULL;
        node->owner = NULL;
        mgr->count--;
        return node;
    }
}

static
client* create_client()
{
    client* cli = malloc(sizeof(client));
    memset(cli, 0, sizeof(*cli));

    return cli;
}

static
void destroy_client(client* cli)
{
    int fd = fevbuff_destroy(cli->evbuff);
    timer_node_delete(cli->owner->current, cli->tnidx);
    cli->owner->current_conn--;
    free(cli);
    close(fd);
    //printf("destroy client fd=%d\n", fd);
}

client_mgr* create_client_mgr()
{
    client_mgr* mgr = malloc(sizeof(client_mgr));
    memset(mgr, 0, sizeof(client_mgr));
    mgr->current = &mgr->tm_main;
    mgr->backup = &mgr->tm_minor;
    mgr->max_open_files = 0;
    mgr->current_conn = 0;

    return mgr;
}

void http_on_timer(fev_state* fev, void* arg)
{
    //printf("on timer\n");
    client_mgr* mgr = (client_mgr*)arg;
    timer_node* node = timer_node_pop(mgr->current);
    if ( !node ) return;

    my_time now;
    get_cur_time(&now);

    while ( node ) {
        int diff = get_diff_time(&node->cli->last_active, &now) / 1000;
        //printf("on timer: fd=%d, diff=%d\n", node->cli->fd, diff);
        if ( diff >= node->timeout ) {
            if ( node->cli->response_complete < node->cli->request_complete ) {
                //printf("send response\n");
                fevbuff_write(node->cli->evbuff, fake_response, sizeof(fake_response) + 1);
                node->cli->response_complete++;
                timer_node_push(mgr->backup, node);
            } else if ( diff > 1000 ) {
                //printf("delete timeout\n");
                destroy_client(node->cli);
            } else {
                timer_node_push(mgr->backup, node);
            }
        } else {
            timer_node_push(mgr->backup, node);
        }

        node = timer_node_pop(mgr->current);
    }

    // swap tmp timer node header and tailer
    timer_mgr* tmp = mgr->current;
    mgr->current = mgr->backup;
    mgr->backup = tmp;
}

void http_read(fev_state* fev, fev_buff* evbuff, void* arg)
{
    int bytes = fevbuff_read(evbuff, NULL, 1024);
    if ( bytes > 0 ) {
        client* cli = (client*)arg;
        get_cur_time(&cli->last_active);
        if ( cli->request_complete ) {
            return;
        }

        char* read_buf = fevbuff_rawget(evbuff);
        int offset = cli->offset;

        while ( offset < bytes-2 ) {
            if ( (read_buf[offset] == read_buf[offset+1] && 
                  read_buf[offset] == '\n') ||
                 (read_buf[offset] == read_buf[offset+2] &&
                  read_buf[offset] == '\n') ) {
                // head parser complete, send response
                cli->request_complete++;
                timer_node* tnode = timer_node_create(cli, 100);
                timer_node_push(cli->owner->current, tnode);
                fevbuff_pop(evbuff, offset+1);
                return;
            }

            offset++;
        }

        cli->offset = offset;
    }
}

void http_error(fev_state* fev, fev_buff* evbuff, void* arg)
{
    //printf("eg error fd=%d\n", ((client*)arg)->fd);
    destroy_client((client*)arg);
}

void http_accept(fev_state* fev, int fd, void* ud)
{
    //printf("accept fd=%d, pid=%d\n", fd, getpid());
    client_mgr* mgr = (client_mgr*)ud;
    if ( fd >= mgr->max_open_files ) {
        printf("fd > max open files, cannot accept pid=%d\n", getpid());
        goto EG_ERROR;
    }

    client* cli = create_client();
    fev_buff* evbuff = fevbuff_new(fev, fd, http_read, http_error, cli);
    if( evbuff ) {
        get_cur_time(&cli->last_active);
        cli->fd = fd;
        cli->evbuff = evbuff;
        cli->owner = mgr;
        cli->owner->current_conn++;
        //printf("fev_buff created fd=%d\n", fd);
    } else {
        printf("cannot create evbuff fd=%d\n", fd);
EG_ERROR:
        close(fd);
    }
}

int init_service(service_arg_t* sargs)
{
    fev = fev_create(sargs->max_queue_len);
    if( !fev ) {
        printf("fev create failed\n");
        exit(1);
    }
    printf("fev create successful\n");

    cli_mgr = create_client_mgr();
    cli_mgr->max_open_files = sargs->max_queue_len;

    fev_listen_info* fli = fev_add_listener(fev, sargs->port, http_accept, cli_mgr);
    if( !fli ) {
        printf("add listener failed\n");
        exit(2);
    }
    printf("add listener successful, bind port is %d\n", sargs->port);

    return 0;
}

int start_service()
{
    fev_timer* resp_timer = fev_add_timer_event(fev, 100000000l, 100000000l,
                                                http_on_timer, cli_mgr);
    if ( !resp_timer ) {
        perror("register timer failed\n");
        exit(1);
    }
    printf("register timer successful\n");
    printf("fev_poll start\n");

    while(1) {
        fev_poll(fev, 10000);
    }

    return 0;
}
