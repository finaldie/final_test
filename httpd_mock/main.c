/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  a simple server used fev
 *
 *        Version:  1.0
 *        Created:  12/05/2011 10:40:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "fev_listener.h"
#include "fev_buff.h"

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

static int current_conn = 0;
static int max_open_files = 0;

struct client;

typedef struct timer_node {
    struct client* cli;
    struct timer_node* prev;
    struct timer_node* next;
} timer_node;

typedef struct {
    int         offset;
    timer_node* tnode;
    fev_buff*   evbuff;
} client;

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
    free(cli);
    close(fd);
    current_conn--;
    //printf("destroy client fd=%d\n", fd);
}

static void eg_read(fev_state* fev, fev_buff* evbuff, void* arg)
{
    int bytes = fevbuff_read(evbuff, NULL, 1024);
    if ( bytes > 0 ) {
        client* cli = (client*)arg;
        char* read_buf = fevbuff_rawget(evbuff);
        int offset = cli->offset;
        while ( offset < bytes-2 ) {
            if ( (read_buf[offset] == read_buf[offset+1] && 
                  read_buf[offset] == '\n') ||
                 (read_buf[offset] == read_buf[offset+2] &&
                  read_buf[offset] == '\n') ) {
                // head parser complete, send response
                fevbuff_write(evbuff, fake_response, sizeof(fake_response) + 1);
                destroy_client(cli);
                return;
            }

            offset++;
        }

        cli->offset = offset;
    }
}

static void eg_error(fev_state* fev, fev_buff* evbuff, void* arg)
{
    printf("eg error\n");
    destroy_client((client*)arg);
}

static void eg_accept(fev_state* fev, int fd)
{
    if ( fd >= max_open_files ) {
        printf("fd > max open files, cannot accept\n");
        goto EG_ERROR;
    }

    fev_buff* evbuff = fevbuff_new(fev, fd, eg_read, eg_error, cli);
    if( evbuff ) {
        client* cli = create_client();
        cli->offset = 0;
        cli->tnode = NULL;
        cli->evbuff = evbuff;
        current_conn++;
        //printf("fev_buff created fd=%d\n", fd);
    } else {
        printf("cannot create evbuff fd=%d\n", fd);
EG_ERROR:
        close(fd);
    }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

int main ( int argc, char *argv[] )
{
    printf("httpd mock pid=%d\n", getpid());

    struct rlimit limits;
    int ret = getrlimit(RLIMIT_NOFILE, &limits);
    if ( ret ) {
        perror("fail to get max open files");
        exit(1);
    }

    max_open_files = (int)limits.rlim_max;
    printf("max open files = %d\n", max_open_files);
    fev_state* fev = fev_create(max_open_files);
    if( !fev ) {
        printf("fev create failed\n");
        exit(1);
    }
    printf("fev create successful\n");

    fev_listen_info* fli = fev_add_listener(fev, 7758, eg_accept);
    if( !fli ) {
        printf("add listener failed\n");
        exit(2);
    }
    printf("add listener successful, bind port is 7758\n");

    printf("fev_poll start\n");
    while(1) {
        fev_poll(fev, 500);
    }

    return EXIT_SUCCESS;
} /* ----------  end of function main  ---------- */
