/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/05/2011 16:26:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "fev_buff.h"
#include "net_core.h"

static int stop = 0;
static int recv_num = 0;

static void client_read(fev_state* fev, fev_buff* evbuff, void* arg)
{
    int n = 0;

    int bytes = fevbuff_read(evbuff, &n, 4);
    if( bytes == 4 ) {
        recv_num++;
        fevbuff_pop(evbuff, 4);
    }
    printf("evbuff read bytes = %d\n", bytes);
}

static void client_error(fev_state* fev, fev_buff* evbuff, void* arg)
{
    int fd = fevbuff_destroy(evbuff);
    close(fd);

    stop = 1;
    printf("total recv num = %d\n", recv_num);
}

static void* client_recv(void* arg)
{
    int fd = *(int*)arg;
    fev_state* fev = fev_create(10);

    fev_buff* evbuff = fevbuff_new(fev, fd, client_read, client_error, NULL);
    if( !evbuff ) {
        printf("evbuff create failed\n");
        exit(0);
    }

    while( !stop ) {
        fev_poll(fev, 500);
    }

    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    if( argc < 3 ) {
        printf("miss ip or port\n");
        exit(0);
    }
    
    char* ip = argv[1];
    int port = atoi(argv[2]);

    int fd = net_conn(ip, port, 1);
    if( fd < 0 ) {
        printf("connect failed\n");
        exit(0);
    }

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, client_recv, &fd);
    if( ret < 0 ){
        printf("create client_recv thread failed\n");
        exit(0);
    }

    int i = 0;
    for (; i< 100000; i++) {
        printf("input enter...\n");
        char input = 0;
        while( (input = getchar()) != 10 );
        net_send_safe(fd, &i, 4);
    }

    printf("send complete\n");
    pthread_join(tid, NULL);

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
