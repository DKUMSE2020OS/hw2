#include "mutex.h"

void* consumer_prod_cons(void* arg) {
	so_t* so = arg;
	int* ret = malloc(sizeof(int));
	int i = 0;
	//int len;
	char* line;

	while (1) {
		pthread_mutex_lock(&so->lock);
		while (so->full[so->cidx] == 0) {
			if (so->signal == 1)
				break;
			pthread_cond_wait(&so->cv, &so->lock);
		}
		if (so->signal == 1) {
			pthread_mutex_unlock(&so->lock);
			pthread_cond_signal(&so->cv);
			break;
		}
		so->linenum++;
		line = so->line[so->cidx];
		//len = strlen(line);
		if (line != NULL) {
			printf("Cons_%x: [%02d:%02d] %s",
				(unsigned int)pthread_self(), i, so->linenum, line);
			free(so->line[so->cidx]);
			i++;
			so->full[so->cidx] = 0;
		}
		else
			so->signal = 1;
		if (so->cidx == 9)
			so->cidx = 0;
		else
			so->cidx++;
		pthread_mutex_unlock(&so->lock);
		pthread_cond_signal(&so->cv);
	}
	printf("Cons: %d lines\n", i);
	*ret = i;
	pthread_exit(ret);
}
