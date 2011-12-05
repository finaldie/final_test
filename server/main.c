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
#include "fev_listener.h"
#include "fev_buff.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

static void eg_read(fev_state* fev, fev_buff* evbuff, void* arg)
{
    char read_buf[20];
    memset(read_buf, 0, 20);

    int bytes = fevbuff_read(evbuff, read_buf, 4);
    if( bytes == 4 ) {
        printf("read data=%d\n", *(int*)read_buf);
    }
    else {
        printf("read data len < 4 : %d\n", bytes);
    }
}

static void eg_error(fev_state* fev, fev_buff* evbuff, void* arg)
{
    printf("eg error\n");
    int fd = fevbuff_destory(evbuff);
    close(fd);
}

static void eg_accept(fev_state* fev, int fd)
{
    fev_buff* evbuff = fevbuff_new(fev, fd, eg_read, eg_error, NULL);
    printf("fev_buff created\n");
}

int main ( int argc, char *argv[] )
{
    fev_state* fev = fev_create(1024);
    if( !fev ) {
        printf("fev create failed\n");
        exit(1);
    }

    fev_listen_info* fli = fev_add_listener(fev, 7758, eg_accept);
    if( !fli ) {
        printf("add listener failedn");
        exit(2);
    }

    while(1) {
        fev_poll(fev, 500);
    }   

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
