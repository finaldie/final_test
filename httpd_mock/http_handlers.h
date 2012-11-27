#ifndef __HTTP_HANDLERS__
#define __HTTP_HANDLERS__

#define FHTTP_MAX_LOG_FILENAME_SIZE 256

typedef struct {
    int max_queue_len;
    int port;
    int workers;
    int min_latency;
    int max_latency;
    int min_response_size;
    int max_response_size;
    int always_chunked;
    int chunk_blocks;
    int chunk_interval;
    int timeout;
    int log_level;
    char log_filename[FHTTP_MAX_LOG_FILENAME_SIZE];
} service_arg_t;

int init_service(service_arg_t*);
int start_service();

#endif
