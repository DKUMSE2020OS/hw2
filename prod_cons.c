#include "mutex.h"

int main(int argc, char* argv[])
{
	pthread_t prod[100];
	pthread_t cons[100];
	int Nprod, Ncons;
	int rc;   long t;
	int* ret;
	int i;
	FILE* rfile;
	if (argc == 1) {
		printf("usage: ./prod_cons <readfile> #Producer #Consumer\n");
		exit(0);
	}
	so_t* share = malloc(sizeof(so_t));
	memset(share, 0, sizeof(so_t));
	rfile = fopen((char*)argv[1], "rb");
	if (rfile == NULL) {
		perror("rfile");
		exit(0);
	}
	if (argv[2] != NULL) {
		Nprod = atoi(argv[2]);
		if (Nprod > 100) Nprod = 100;
		if (Nprod == 0) Nprod = 1;
	}
	else Nprod = 1;
	if (argv[3] != NULL) {
		Ncons = atoi(argv[3]);
		if (Ncons > 100) Ncons = 100;
		if (Ncons == 0) Ncons = 1;
	}
	else Ncons = 1;

	share->rfile = rfile;
	for (i = 0; i < 10; i++) {
		share->line[i] = NULL;
	}
	pthread_mutex_init(&share->lock, NULL);
	for (i = 0; i < Nprod; i++)
		pthread_create(&prod[i], NULL, producer, share);
	for (i = 0; i < Ncons; i++)
		pthread_create(&cons[i], NULL, consumer_prod_cons, share);
	printf("main continuing\n");

	for (i = 0; i < Ncons; i++) {
		rc = pthread_join(cons[i], (void**)&ret);
		printf("main: consumer_%d joined with %d\n", i, *ret);
	}
	for (i = 0; i < Nprod; i++) {
		rc = pthread_join(prod[i], (void**)&ret);
		printf("main: producer_%d joined with %d\n", i, *ret);
	}
	pthread_exit(NULL);
	exit(0);
}
