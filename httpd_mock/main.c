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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>

#include "http_handlers.h"

// global vars
int max_open_files = 0;

typedef struct {
    int max_queue_len;
    int port;
} service_arg;

//static
//void* service_thread(void* arg)
//{
//    service_arg* svc_arg = (service_arg*)arg;
//    start_service(svc_arg->max_queue_len, svc_arg->port);
//
//    return NULL;
//}

int set_cpu_mask(int cpu_index)                                               {
    cpu_set_t mask;
    /*  CPU_ZERO initializes all the bits in the mask to zero. */
    CPU_ZERO( &mask );
    /*  CPU_SET sets only the bit corresponding to cpu. */
    CPU_SET( cpu_index, &mask );
    /*  sched_setaffinity returns 0 in success */
    if( sched_setaffinity( 0, sizeof(mask), &mask ) == -1 ) {
        printf("WARNING: Could not set CPU Affinity, continuing...\n");
        return 1;
    }
    
    return 0;
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
    //int per_thread_queue_len = max_open_files / 4;
    //pthread_t t[4];
    //int i = 0;
    //service_arg arg = {per_thread_queue_len, 7758};
    //for ( ; i<4; i++ ) {
    //    pthread_create(&t[i], NULL, service_thread, &arg);
    //}

    //for ( i=0; i<4; i++ ) {
    //    pthread_join(t[i], NULL);
    //}

    init_service(max_open_files, 7758);

    int i = 0;
    while ( i < 0 ) {
        int ret = fork();
        if ( ret == 0 ) {
            break;
        }
        i++;
    }

    printf("start service pid=%d, ppid=%d\n", getpid(), getppid());
    //if ( set_cpu_mask(i) ) {
    //    printf("set cpu mask for cpuid=%d failed\n", i);
    //}
    start_service();
    return EXIT_SUCCESS;
} /* ----------  end of function main  ---------- */
