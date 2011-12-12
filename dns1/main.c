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
#include "net_core.h"
#include "fev.h"
#include "firedns.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

static void on_dns(fev_state* fev, int fd, int mask, void* arg)
{
    struct in_addr addr4;
    char* res = firedns_getresult(fd);
    if( !res ) {
        printf("get result is nil\n");
        exit(0);
    }

    memcpy(&addr4, res, sizeof(struct in_addr));
    printf("get res ip4=%s\n", firedns_ntoa4(&addr4));
}

int main ( int argc, char *argv[] )
{
    char* host = "www.csdn.net";
    char* ip = "117.79.157.242";
    int fd = -1;
    net_conn_a(ip, 80, &fd);
    printf("fd = %d\n", fd);

    int fd1 = firedns_getip4(host);
    if( fd1 == -1 ) {
        printf("error request for get ip4\n");
        exit(0);
    }
    printf("fd1=%d\n");
    net_conn_a(ip, 80, &fd);
    printf("fd = %d\n", fd);

    fd1 = firedns_getip4(host);
    if( fd1 == -1 ) {
        printf("error request for get ip4\n");
        exit(0);
    }
    printf("fd1=%d\n");

    /* 
    fev_state* fev = fev_create(1024);
    if( !fev ) {
        printf("fev create failed\n");
        exit(1);
    }
    printf("fev create successful\n");

    int ret = fev_reg_event(fev, fd, FEV_READ, on_dns, NULL, NULL);
    if( ret ) {
        printf("add event failed\n");
        exit(0);
    }

    printf("fev_poll start\n");
    while(1) {
        fev_poll(fev, 500);
    }   
    */

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
