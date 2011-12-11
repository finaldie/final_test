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
    if( argc != 2 ) {
        printf("need ip4\n");
        exit(0);
    }

    int fd = firedns_getip4(argv[1]);
    if( fd == -1 ) {
        printf("error request for get ip4\n");
        exit(0);
    }

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

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
