// Operating System HW2-1 basic_word_count.c 
// 32163122 Minseo Lee (Github: minseodaily)
// Copyright 2020 All Rights Reserved by minseodaily

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct sharedobject {
	FILE *rfile;
	char *line[100];
	pthread_mutex_t lock;
	pthread_cond_t cv;
	int full[100];
} so_t;

void *producer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int pindex = 0;

	while (1) {
		pthread_mutex_lock(&so->lock);

		while(so->full[pindex] == 1) {
			pthread_cond_wait(&so->cv, &so->lock);
		}

		read = getdelim(&line, &len, '\n', rfile);
		
		if (read == -1) {
			so->full[pindex] = 1;
			so->line[pindex] = NULL;
		} else {
			so->line[pindex] = strdup(line);
			i++;
			so->full[pindex] = 1;
		}

		pthread_mutex_unlock(&so->lock);
		pthread_cond_signal(&so->cv);

		if (pindex == 99) {
			pindex = 0;
		} else {
			pindex++;
		}
	
		if(read == -1) break;
	}
	free(line);
	*ret = i;
	pthread_exit(ret);
}

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	char *line;
	int cindex;

	while (1) {
		pthread_mutex_lock(&so->lock);

		while(so->full[cindex] == 0) {
			pthread_cond_wait(&so->cv, &so->lock);
		}

		line = so->line[cindex];

		if (line != NULL) {
			free(so->line[cindex]);
			i++;
			so->full[cindex] = 0;
		}

		pthread_mutex_unlock(&so->lock);
		pthread_cond_signal(&so->cv);

		if (cindex == 99) {
			cindex = 0;
		} else {
			cindex++;
		}

		if(line == NULL) break;
	}
	*ret = i;
	pthread_exit(ret);
}


int main (int argc, char *argv[])
{
	pthread_t prod[100];
	pthread_t cons[100];
	int Nprod, Ncons;
	int rc;   long t;
	int *ret;
	int i;
	FILE *rfile;
	if (argc == 1) {
		printf("usage: ./prod_cons <readfile> #Producer #Consumer\n");
		exit (0);
	}
	so_t *share = malloc(sizeof(so_t));
	memset(share, 0, sizeof(so_t));
	rfile = fopen((char *) argv[1], "r");
	if (rfile == NULL) {
		perror("rfile");
		exit(0);
	}
	if (argv[2] != NULL) {
		Nprod = atoi(argv[2]);
		if (Nprod > 100) Nprod = 100;
		if (Nprod == 0) Nprod = 1;
	} else Nprod = 1;
	if (argv[3] != NULL) {
		Ncons = atoi(argv[3]);
		if (Ncons > 100) Ncons = 100;
		if (Ncons == 0) Ncons = 1;
	} else Ncons = 1;

	share->rfile = rfile;
	for(i=0; i<100; i++) {
		share->line[i] = NULL;
	}

	pthread_mutex_init(&share->lock, NULL);
	for (i = 0 ; i < Nprod ; i++)
		pthread_create(&prod[i], NULL, producer, share);
	for (i = 0 ; i < Ncons ; i++)
		pthread_create(&cons[i], NULL, consumer, share);
	printf("main continuing\n");

	for (i = 0 ; i < Ncons ; i++) {
		rc = pthread_join(cons[i], (void **) &ret);
		printf("main: consumer_%d joined with %d\n", i, *ret);
	}
	for (i = 0 ; i < Nprod ; i++) {
		rc = pthread_join(prod[i], (void **) &ret);
		printf("main: producer_%d joined with %d\n", i, *ret);
	}
	pthread_exit(NULL);
	exit(0);
}

