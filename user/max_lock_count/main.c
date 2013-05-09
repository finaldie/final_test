#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char** argv)
{
	int count = 0;
	while(1) {
		pthread_mutex_t mt;
		if ( pthread_mutex_init(&mt, NULL) ) {
			perror("init:");
			exit(1);
		}
		if ( pthread_mutex_lock(&mt) ) {
			perror("lock:");
			exit(1);
		}
		count++;
		printf("current lock number = %d\n", count);
	}
	return 0;
}
