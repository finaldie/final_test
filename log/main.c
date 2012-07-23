#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "log_inc.h"
#include "tu_inc.h"

static flogger* log_handler = NULL;

static
void* write_log(void* arg)
{
    printf("writing thread id = %lu startup\n", pthread_self());
    int num = *((int*)arg);

    my_time start, end;
    get_cur_time(&start);
    int i = 0;
    for ( i = 0; i < num; i++ ) {
        FLOG_DEBUG(log_handler, "debug log test");
    }
    get_cur_time(&end);

    int diff_usec = get_diff_time(&start, &end);
    printf("tid=%lu, call interface time cost (usec):%d\n", pthread_self(), diff_usec);

    return NULL;
}

static
void do_test(int num, int thread_num, FLOG_MODE mode)
{
    flog_set_mode(mode);
    flog_set_flush_interval(2);
    flog_set_level(LOG_LEVEL_DEBUG);

    my_time start_time, end_time;
    get_cur_time(&start_time);
    int end = num / thread_num;
    pthread_t tid[thread_num];
    int i = 0;
    for ( i = 0; i < thread_num; i++ ) {
        pthread_create(&tid[i], NULL, write_log, (void*)&end);
    }

    for ( i = 0; i < thread_num; i++ ) {
        pthread_join(tid[i], NULL);
    }

    get_cur_time(&end_time);
    int diff_usec = get_diff_time(&start_time, &end_time);
    printf("pid=%d, tid=%lu, call interface time cost (usec):%d\n", getpid(), pthread_self(), diff_usec);
}

static
void test_single_sync(int num)
{
    log_handler = flog_create("sync_single_thread");
    printf("[SYNC]start single testing...\n");
    do_test(num, 1, FLOG_SYNC_MODE);
    sleep(2);
    printf("[SYNC]end single testing\n\n");
}

static
void test_multi_sync(int num, int thread_num)
{
    log_handler = flog_create("sync_multithread");
    printf("[SYNC]start multip testing ( totally, we start %d threads for testing)...\n", thread_num);
    do_test(num, thread_num, FLOG_SYNC_MODE);
    sleep(4);
    printf("[SYNC]end multip testing\n\n");
}

static
void test_single_async(int num)
{
    log_handler = flog_create("async_single_thread");
    printf("[ASYNC]start single testing...\n");
    do_test(num, 1, FLOG_ASYNC_MODE);
    sleep(2);
    printf("[ASYNC]end single testing\n\n");
}

static
void test_multi_async(int num, int thread_num)
{
    log_handler = flog_create("async_multithread");
    printf("[ASYNC]start multip testing ( totally, we start %d threads for testing)...\n", thread_num);
    do_test(num, thread_num, FLOG_ASYNC_MODE);
    sleep(6);
    printf("[ASYNC]end multip testing\n\n");
}

// cmd: ./test 100000 3
// note: first param - count of log msg
//       second param - how many threads we start
int main(int argc, char** argv)
{
    if ( argc < 2 ) {
        printf("please input a number for logging count\n");
        printf("cmd format: ./test msg_count [ thread_num ] [ mode ]\n");
        printf("param description:\n");
        printf("msg_count(): must > 0\n");
        printf("thread_num(optional): must > 0, default value is 2\n");
        printf("mode(optional): default value is 4\n");
        printf("  `---- 0: only test single thread sync writing\n");
        printf("  `---- 1: only test multithread sync writin\n");
        printf("  `---- 2: only test single thread async wrting\n");
        printf("  `---- 3: only test mulithread async writing\n");
        printf("  `---- 4: test all\n");
        printf("\ncmd example:\n");
        printf("./test 100000\n");
        printf("./test 100000 2\n");
        printf("./test 100000 2 3\n");
        exit(1);
    }

    int num = atoi(argv[1]);
    if ( num <= 0 ) {
        printf("invalid msg_count, must > 0\n");
        exit(1);
    }

    // default thread num = 2
    int thread_num = 2;
    // get thread num
    if ( argc >= 3 ) {
        thread_num = atoi(argv[2]);
        if ( thread_num <= 0 ) {
            printf("wrong number of thread_num, must > 0\n");
            exit(1);
        }
    }

    // 0: only test single thread sync writing
    // 1: only test multithread sync writin
    // 2: only test single thread async wrting
    // 3: only test mulithread async writing
    // 4: test all
    int mode = 4;
    if ( argc == 4 ) {
        mode = atoi(argv[3]);
        if ( mode < 0 || mode > 4 ) {
            printf("wrong mode, mode must within 0-4\n");
            exit(1);
        }
    }

    printf("startup mode = %d ..\n", mode);
    switch ( mode ) {
        case 0:
            {
                test_single_sync(num);
                break;
            }
        case 1:
            {
                test_multi_sync(num, thread_num);
                break;
            }
        case 2:
            {
                test_single_async(num);
                break;
            }
        case 3:
            {
                test_multi_async(num, thread_num);
                break;
            }
        case 4:
            {
                test_single_sync(num);
                test_multi_sync(num, thread_num);
                test_single_async(num);
                test_multi_async(num, thread_num);
                break;
            }
    }

    return 0;
}
