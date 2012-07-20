#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "log_inc.h"

static flogger* log_handler = NULL;

static
void* write_log(void* arg)
{
    printf("writing thread id = %lu startup\n", pthread_self());
    int num = *((int*)arg);

    for ( int i = 0; i < num; i++ ) {
        FLOG_DEBUG(log_handler, "debug log test");
    }

    return NULL;
}

// cmd: ./test 100000 3
// note: first param - count of log msg
//       second param - how many threads we start
int main(int argc, char** argv)
{
    if ( argc < 2 ) {
        printf("please input a number for logging count\n");
        printf("cmd format: ./test msg_count [ thread_num ]\n");
        exit(1);
    }

    int num = atoi(argv[1]);

    // default thread num = 2
    int thread_num = 2;
    // get thread num
    if ( argc == 3 ) {
        thread_num = atoi(argv[2]);
    }

    printf("startup...\n");
    log_handler = flog_create("single_thread");
    flog_set_mode(FLOG_ASYNC_MODE);
    flog_set_flush_interval(1);
    sleep(1);
    printf("start single testing...\n");

    flog_set_level(LOG_LEVEL_DEBUG);
    for ( int i = 0; i < num; i++ ) {
        FLOG_DEBUG(log_handler, "debug log test");
    }

    sleep(2);

    printf("start multip testing ( totally, we start %d threads for testing)...\n", thread_num);
    log_handler = flog_create("multip_thread");
    pthread_t tid[thread_num];
    int end = num / thread_num;

    for ( int i = 0; i < thread_num; i++ ) {
        pthread_create(&tid[i], NULL, write_log, (void*)&end);
    }

    for ( int i = 0; i < thread_num; i++ ) {
        pthread_join(tid[i], NULL);
    }

    sleep(2);
}
