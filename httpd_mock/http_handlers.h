#ifndef __HTTP_HANDLERS__
#define __HTTP_HANDLERS__

typedef struct {
    int max_queue_len;
    int port;
    int workers;
} service_arg_t;

int init_service(service_arg_t*);
int start_service();

#endif
