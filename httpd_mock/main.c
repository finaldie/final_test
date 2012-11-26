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

#include "read_conf.h"
#include "http_handlers.h"

// global vars
int max_open_files = 0;

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

static
int init_service_args(service_arg_t* sargs)
{
    if ( !sargs ) return 1;

    memset(sargs, 0, sizeof(service_arg_t));
    sargs->max_queue_len = 1024;
    sargs->port = 80;
    sargs->workers = 1;
    sargs->min_latency = 100;
    sargs->max_latency = 100;
    sargs->min_response_size = 100;
    sargs->max_response_size = 100;
    sargs->always_chunked = 1;
    sargs->timeout = 1000;

    return 0;
}

void read_config(const char* filename, service_arg_t* sargs)
{
    // init args
    init_service_args(sargs);

    void _read_pairs(char* key, char* value) {
        if ( strcmp(key, "listen_port") == 0 ) {
            sargs->port = atoi(value);
        } else if ( strcmp(key, "max_connection") == 0 ) {
            sargs->max_queue_len = atoi(value);
        } else if ( strcmp(key, "workers") == 0 ) {
            sargs->workers = atoi(value);
        }
    }

    int ret = GenConfig((char*)filename, _read_pairs);
    if ( ret ) {
        printf("configuration error, please check it\n");
        exit(1);
    }
}

int checkServiceArgs(service_arg_t* sargs)
{
    // check max open files
    struct rlimit limits;
    int ret = getrlimit(RLIMIT_NOFILE, &limits);
    if ( ret ) {
        perror("fail to get max open files");
        exit(1);
    }

    max_open_files = (int)limits.rlim_max;
    printf("max open files = %d\n", max_open_files);
    if ( sargs->max_queue_len > max_open_files ) {
        sargs->max_queue_len = max_open_files;
    }

    // check workers
    if ( sargs->workers <= 0 ) {
        sargs->workers = 1;
    }

    // check port
    if ( sargs->port <= 0 || sargs->port > 65535 ) {
        printf("invalid port number [%d], must in [1-65535]\n", sargs->port);
        exit(1);
    }

    printf("args:\n");
    printf("  \\_ listen_port : %d\n", sargs->port);
    printf("  \\_ workers : %d\n", sargs->workers);
    printf("  \\_ max_connection : %d\n", sargs->max_queue_len);
    printf("  \\_ min_latency : %d\n", sargs->min_latency);
    printf("  \\_ max_latency : %d\n", sargs->max_latency);
    printf("  \\_ min_response_size : %d\n", sargs->min_response_size);
    printf("  \\_ max_response_size : %d\n", sargs->max_response_size);
    printf("  \\_ always_chunked : %d\n", sargs->always_chunked);
    printf("  \\_ timeout : %d\n", sargs->timeout);
    printf("\n");

    return 0;
}

void printUsage()
{
    printf("usage:\n");
    printf("  \\_ http_mock [-p port] [-c config_file]\n");
    printf("args:\n");
    printf("  \\_ p : listen port number\n");
    printf("  \\_ c : configuration file, use the absolute path\n");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

int main ( int argc, char *argv[] )
{
    service_arg_t service_arg;
    init_service_args(&service_arg);

    printf("httpd mock pid=%d\n", getpid());

    if ( argc > 1 ) {
        char opt;
        while ((opt = getopt(argc, argv, "p:c:")) != -1) {
            switch (opt) {
                case 'c':
                    read_config(optarg, &service_arg);
                    goto prepare_start;
                case 'p':
                    service_arg.port = atoi(optarg);
                    break;
                default:
                    printUsage();
                    exit(0);
            }
        }
    } else {
        printUsage();
        exit(0);
    }

prepare_start:
    checkServiceArgs(&service_arg);

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

    init_service(&service_arg);

    int i = 0;
    while ( i < (service_arg.workers - 1) ) {
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
