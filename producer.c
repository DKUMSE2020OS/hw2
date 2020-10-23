#include "mutex.h"

void* producer(void* arg) {
	so_t* so = arg;
	int* ret = malloc(sizeof(int));
	FILE* rfile = so->rfile;
	int i = 0;
	char* line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int pidx = 0;

	while (1) {
		//grab mutex lock
		//check condvar
		pthread_mutex_lock(&so->lock);
		while (so->full[pidx] == 1) {
			pthread_cond_wait(&so->cv, &so->lock);
		}
		//enter CS
		read = getdelim(&line, &len, '\n', rfile);
		if (read == -1) {
			so->full[pidx] = 1;
			so->line[pidx] = NULL;
		}
		else {
			so->full[pidx] = 1;
			so->line[pidx] = strdup(line);      /* share the line */
			i++;
		}
		if (pidx == 9)
			pidx = 0;
		else
			pidx++;
		//release lock
		//signal condvar
		pthread_mutex_unlock(&so->lock);
		pthread_cond_signal(&so->cv);
		if (read == -1) break;
	}
	free(line);
	printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
	*ret = i;
	pthread_exit(ret);
}
