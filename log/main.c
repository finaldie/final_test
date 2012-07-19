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

int main(int argc, char** argv)
{
    if ( argc < 2 ) {
        printf("please input a number for logging count\n");
        exit(1);
    }

    int num = atoi(argv[1]);

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

    printf("start multip testing...\n");
    log_handler = flog_create("multip_thread");
    pthread_t tid1, tid2, tid3;
    int end = num / 3;
    pthread_create(&tid1, NULL, write_log, (void*)&end);
    pthread_create(&tid2, NULL, write_log, (void*)&end);
    pthread_create(&tid3, NULL, write_log, (void*)&end);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    sleep(2);
}
