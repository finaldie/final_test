#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "log_inc.h"
#include "tu_inc.h"

// you need change the two marcos as below
#define MAX_LOG_SIZE             500
#define MAX_BUFF_SIZE_PER_THREAD (1024 * 1024 * 200)

static log_file_t* log_handler = NULL;
static char log_str[MAX_LOG_SIZE];
static int buff_full_count = 0;
static LOG_MODE log_mode;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void get_log_event(LOG_EVENT event)
{
    if ( event == LOG_EVENT_BUFF_FULL ) {
        pthread_mutex_lock(&lock);
        buff_full_count++;
        pthread_mutex_unlock(&lock);
    }

    if ( event == LOG_EVENT_USER_BUFFER_RELEASED ) {
        printf("get_log_event(tid=%lu): event_id=%d\n", pthread_self(), event);
    }
}

static
void* write_log(void* arg)
{
    printf("writing thread id = %lu startup\n", pthread_self());
    int num = *((int*)arg);
    int last_miss_count = buff_full_count;
    // divide total msg into some groups
    int max_num_per_group = 5000;
    int group = num / max_num_per_group;
    int sleep_step = max_num_per_group;
    printf("tid=%lu, max_num_per_group:%d, group:%d, last_miss_count:%d, sleep_step:%d\n",
            pthread_self(), max_num_per_group, group, last_miss_count, sleep_step);

    my_time start, end;
    get_cur_time(&start);
    int i = 0, j = 0;
    for ( i = 0; i < group; i++ ) {
        if ( log_mode == LOG_ASYNC_MODE ) {
            last_miss_count = buff_full_count;
        }

        int step = 0;
        for ( j = 0; j < max_num_per_group; ++j, ++step ) {
            FLOG_DEBUG(log_handler, log_str);
            //log_file_write(log_handler, NULL, 0, log_str, MAX_LOG_SIZE - 1);

            if ( log_mode == LOG_ASYNC_MODE && (step == sleep_step) ) {
                step = 0;
                usleep(1);
            }
        }

        if ( log_mode == LOG_ASYNC_MODE ) {
            int current_miss_count = buff_full_count;
            int diff = current_miss_count - last_miss_count;

            if ( diff > 0 ) {
                sleep_step /= 2;
                if ( sleep_step == 0 ) sleep_step = 1;
            } else {
                sleep_step += 250;
                if ( sleep_step > max_num_per_group ) sleep_step = max_num_per_group;
            }

            if ( diff ) {
                printf("tid=%lu, group_id:%d, last_miss_count:%d, diff:%d, sleep_step:%d\n",
                        pthread_self(), i, last_miss_count, diff, sleep_step);
            }
        }
    }

    get_cur_time(&end);
    int diff_usec = get_diff_time(&start, &end);
    printf("tid=%lu, call interface time cost (usec):%d, writen msg:%d, final:%f count/s\n", 
            pthread_self(), diff_usec, num, (double)num / ((double)diff_usec / 1000000));

    pthread_exit(NULL);
}

static
void do_test(int num, int thread_num)
{
    flog_set_mode(log_mode);
    flog_set_flush_interval(2);
    flog_set_level(LOG_LEVEL_DEBUG);
    flog_set_buffer_size(MAX_BUFF_SIZE_PER_THREAD);
    if ( log_mode == LOG_ASYNC_MODE ) {
        printf("current buffer size per-thread = %lu\n", flog_get_buffer_size());
    }
    flog_register_event_callback(get_log_event);
    buff_full_count = 0;
    sleep(1);

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
    printf("pid=%d, tid=%lu, call interface time cost (usec):%d write_msg:%d miss_msg:%d miss_rate:%f final:%f count/s\n",
            getpid(), pthread_self(), diff_usec, num, buff_full_count, (double)buff_full_count/(double)num, (double)num/((double)diff_usec/1000000));
}

static
void test_single_sync(int num)
{
    log_handler = flog_create("sync_single_thread");
    printf("[SYNC]start single testing...\n");
    log_mode = LOG_SYNC_MODE;
    do_test(num, 1);
    sleep(2);
    printf("[SYNC]end single testing\n\n");
}

static
void test_multi_sync(int num, int thread_num)
{
    log_handler = flog_create("sync_multithread");
    printf("[SYNC]start multip testing ( totally, we start %d threads for testing)...\n", thread_num);
    log_mode = LOG_SYNC_MODE;
    do_test(num, thread_num);
    sleep(4);
    printf("[SYNC]end multip testing\n\n");
}

static
void test_single_async(int num)
{
    log_handler = flog_create("async_single_thread");
    printf("[ASYNC]start single testing...\n");
    log_mode = LOG_ASYNC_MODE;
    do_test(num, 1);
    sleep(2);
    printf("[ASYNC]end single testing\n\n");
}

static
void test_multi_async(int num, int thread_num)
{
    log_handler = flog_create("async_multithread");
    printf("[ASYNC]start multip testing ( totally, we start %d threads for testing)...\n", thread_num);
    log_mode = LOG_ASYNC_MODE;
    do_test(num, thread_num);
    sleep(6);
    printf("[ASYNC]end multip testing\n\n");
}

size_t critical_test()
{
    char log[MAX_LOG_SIZE];
    memset(log, 0, MAX_LOG_SIZE);
    memset(log, 97, MAX_LOG_SIZE - 1);
    log[MAX_LOG_SIZE-2] = '\n';
    FILE* f = fopen("crit_test.log", "a");
    if ( !f ) {
        printf("cannot create/open crit_test.log for testing\n");
        exit(1);
    }

    my_time start_time, end_time;
    get_cur_time(&start_time);

    int i = 0;
    int msg_count = 20000000;
    for ( i = 0; i < msg_count; ++i ) {
        fwrite_unlocked(log, 1, MAX_LOG_SIZE-1, f);
    }

    get_cur_time(&end_time);
    int diff_usec = get_diff_time(&start_time, &end_time);
    fclose(f);

    printf("total cost (usec):%d, total msg:%d count, per-msg_size:%d bytes\n",
            diff_usec, msg_count, MAX_LOG_SIZE);
    double max = (double)msg_count / ((double)diff_usec/(double)1000000 );
    return (size_t)max;
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

    memset(log_str, 0, MAX_LOG_SIZE);
    memset(log_str, 97, MAX_LOG_SIZE-1);

    //printf("start critical test...\n");
    //size_t max = critical_test();
    //printf("start critical test...end, the max writing ability is: %lu count/s\n", max);

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
