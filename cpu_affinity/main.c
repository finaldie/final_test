/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  for testing cpu affinity effect
 *
 *        Version:  1.0
 *        Created:  10/31/2012 13:15:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *
 * =====================================================================================
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>


static int ARRAY_SIZE = 0;
static int IS_SET = 0;
static int CPU_CORES = 0;
static int* nums_array = NULL;

void init_nums_array()
{
    nums_array = malloc(ARRAY_SIZE * sizeof(int));
    if ( !nums_array ) {
        printf("init num array : malloc failed\n");
        exit(1);
    }

    memset(nums_array, 1, ARRAY_SIZE * sizeof(int));
}

int do_cpu_expensive_op(int loop) {
    int res = 0;
    int i = 0, j = 0;
    for ( ; i < loop; i++ ) {
        for (j=0; j<ARRAY_SIZE; j++) {
            res *= nums_array[j];
        }
    }

    return res;
}

int check_cpu_expensive_op(int res)
{
    return 1;
}

int do_thread_task()
{
    int ret = 1;
    /*  Now we have a single thread bound to each cpu on the system */
    int computation_res = do_cpu_expensive_op(1000);
    cpu_set_t mycpuid;
    sched_getaffinity(0, sizeof(mycpuid), &mycpuid);
    if ( check_cpu_expensive_op(computation_res) ) {
        printf("SUCCESS: Thread completed, and PASSED integrity check!\n");
        ret = 1;
    } else {
        printf("FAILURE: Thread failed integrity check!\n");
        ret = 0;
    }

    return ret;
}

int set_cpu_mask(int cpu_index)
{
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

int set_cpu_mask_forthread(int cpu_index)
{
    cpu_set_t mask;
    /*  CPU_ZERO initializes all the bits in the mask to zero. */
    CPU_ZERO( &mask );
    /*  CPU_SET sets only the bit corresponding to cpu. */
    CPU_SET( cpu_index % CPU_CORES, &mask );
    /*  sched_setaffinity returns 0 in success */
    if( pthread_setaffinity_np( pthread_self(), sizeof(mask), &mask ) == -1 ) {
        printf("WARNING: Could not set CPU Affinity, continuing...\n");
        return 1;
    }

    return 0;

}

/*  This method will create threads, then bind each to its own cpu. */
int do_cpu_stress(int numthreads)
{
    int ret = 1;
    int created_thread = 0;

    /*  We need a thread for each cpu we have... */
    while ( created_thread < numthreads - 1 ) {
        int mypid = fork();
        if (mypid == 0) { /*  Child process */
            printf("\tCreating Child Thread: #%i\n", created_thread);
            break;
        } else { /*  Only parent executes this */
            /*  Continue looping until we spawned enough threads! */;
            created_thread++;
        }
    }

    /*  NOTE: All threads execute code from here down! */
    if ( IS_SET ) {
        if ( set_cpu_mask(created_thread) ) {
            printf("set cpu mask failed, cpu index = %d\n", created_thread);
            exit(1);
        }
    }

    ret = do_thread_task();

    return ret;
}

void* _do_thread_task(void* arg)
{
    int thread_index = *(int*)arg;
    if ( IS_SET ) {
        if ( set_cpu_mask_forthread(thread_index) ) {
            printf("set cpu mask failed, cpu index = %d\n", thread_index);
            exit(1);
        }
    }

    do_thread_task();
    return NULL;
}

int do_cpu_thread_stress(int numthreads)
{
    pthread_t tids[numthreads];
    int indexs[numthreads];
    int i = 0;
    for ( i=0 ; i < numthreads; i++ ) {
        indexs[i] = i;
        pthread_create(&tids[i], NULL, _do_thread_task, &indexs[i]);
    }

    for ( i=0; i < numthreads; i++ ) {
        pthread_join(tids[i], NULL);
    }

    printf("stress test finished\n");
    return 1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    if ( argc < 3 ) {
        printf("usage : ./binary thread_num array_size [is_set_mask:0]\n");
        printf(" `- example: ./test 4 1000000\n");
        printf(" `- example: ./test 4 1000000 1\n");
        exit(0);
    }

    int thread_num = atoi(argv[1]);
    if ( thread_num < 1 ) {
        printf("thread_num must > 0\n");
        exit(0);
    }

    int size = atoi(argv[2]);
    if ( size < 1 ) {
        printf("array_size must > 0\n");
        exit(0);
    }
    ARRAY_SIZE = size;

    if ( argc == 4 ) {
        int is_set = atoi(argv[3]);
        if ( is_set ) IS_SET = 1;
    }

    printf("The thread_num=[%d] array_size=[%d], is_set_mask=[%d]\n", thread_num, ARRAY_SIZE, IS_SET);
    init_nums_array();
    CPU_CORES = sysconf(_SC_NPROCESSORS_CONF);
    do_cpu_thread_stress(thread_num);
    return EXIT_SUCCESS;
}   /* ----------  end of function main  ---------- */
