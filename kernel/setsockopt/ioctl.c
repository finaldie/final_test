/*
 *  ioctl.c − the process to use ioctl's to control the kernel module
 *
 *  Until now we could have used cat for input and output.  But now
 *  we need to do ioctl's, which require writing our own process.
 */
/*
 * device specifics, such as ioctl numbers and the
 * major device file.
 */
#include "chardev.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>              /* open */
#include <unistd.h>             /* exit */
#include <sys/ioctl.h>          /* ioctl */
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define fake_response_header \
"HTTP/1.1 200 OK\r\n" \
"Date: Tue, 13 Nov 2012 13:21:30 GMT\r\n" \
"Server: Http Mock\r\n" \
"Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n" \
"Content-Length: 0\r\n" \
"Connection: Keep-Alive\r\n" \
"Content-Type: text/html\r\n" \
"\r\n"

/*
 * Functions for the ioctl calls
 */
void ioctl_set_msg(int file_desc, char *message)
{
        int ret_val;
        ret_val = ioctl(file_desc, IOCTL_SET_MSG, message);
	if (ret_val < 0) {
        	printf("ioctl_set_msg failed:%d\n", ret_val);
        	exit(-1);
	}
}
	
void ioctl_get_msg(int file_desc)
{
	int ret_val;
	char message[100];
	/*
	 * Warning − this is dangerous because we don't tell
	 * the kernel how far it's allowed to write, so it
	 * might overflow the buffer. In a real production
	 * program, we would have used two ioctls − one to tell
	 * the kernel the buffer length and another to give
	 * it the buffer to fill
	 */
	ret_val = ioctl(file_desc, IOCTL_GET_MSG, message);
	if (ret_val < 0) {
	        printf("ioctl_get_msg failed:%d\n", ret_val);
	        exit(-1);
	}
	printf("get_msg message:%s\n", message);
}

void ioctl_get_nth_byte(int file_desc)
{
	int i; char c;
	printf("get_nth_byte message:");
	i = 0;
	do {
		c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);
		if (c < 0) {
		        printf("ioctl_get_nth_byte failed at the %d'th byte:\n", i);
			exit(-1);
		}

		putchar(c);
	} while (c != 0);
	putchar('\n');	
}

int create_listen(int chardev_fd, int port)
{
    int listen_fd;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > listen_fd) return -1;
    ioctl(chardev_fd, IOCTL_SET_SOCK_FORCEUSE, listen_fd);

    if (0 > bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr))) {
        close(listen_fd);
        return -1;
    }

    if (0 > listen(listen_fd, 30000)) {
        close(listen_fd);
        return -1;
    }                                                                           
    return listen_fd;
}

void* worker(void* arg)
{
	int chardev_fd = *(int*)arg;
	int listen_fd = create_listen(chardev_fd, 7760);
	printf("listen fd = %d\n", listen_fd);

	int loop = 0;
	char tmpbuf[1000];
	while(1) {
		struct sockaddr addr;
		socklen_t len;
		//printf("ready to accept newfd, tid=%lu listen_fd=%d\n", pthread_self(), listen_fd);
		int newfd = accept(listen_fd, &addr, &len);
		printf("accept new fd %d in thread:%lu, listen_fd=%d, err=%d, %s\n", newfd, pthread_self(), listen_fd, errno, strerror(errno));
		//close(newfd);
		//if ( loop == 500 ) {
		//	loop = 0;
		//	sleep(5);
		//}

		memset(tmpbuf, 0, 1000);
		read(newfd, tmpbuf, 1000);
		printf("tmpbuf=%s\n", tmpbuf);
		send(newfd, fake_response_header, sizeof(fake_response_header) - 1, MSG_NOSIGNAL);
		loop++;
		close(newfd);
	}
	return NULL;
}

void test_setportreuse(int chardev_fd)
{
	pthread_t t1, t2;
	pthread_create(&t1, NULL, worker, &chardev_fd);
	pthread_create(&t2, NULL, worker, &chardev_fd);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}

/*
 * Main − Call the ioctl functions
 */
int main(int argc, char** argv)
{
	int file_desc;
	char *msg = "Message passed by ioctl\n";
	file_desc = open("/dev/" DEVICE_FILE_NAME, 0);
	if (file_desc < 0) {
		printf("Can't open device file: %s->%s\n", DEVICE_FILE_NAME, strerror(errno));
		exit(-1);
	}
	//ioctl_get_nth_byte(file_desc);
	//ioctl_get_msg(file_desc);
	//ioctl_set_msg(file_desc, msg);
	test_setportreuse(file_desc);
	close(file_desc);
	return 0;
}
