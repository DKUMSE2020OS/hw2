#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct sharedobject {
	FILE *rfile;
	int linenum;
	char *line[50];
	pthread_mutex_t lock; //for mutual exclusion
	pthread_cond_t cv; //condvar for checking full
	int full[50];
	int Cindex;
	int EXITSIG;
} so_t;

void *producer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int Pindex = 0;

	while (1) {
		read = getdelim(&line, &len, '\n', rfile);
		pthread_mutex_lock(&so->lock);

//------------start of prod lock---------------------------------------------
		while(so->full[Pindex] == 1){
			pthread_cond_wait(&so->cv, &so->lock);
		}
		if (read == -1){
			so->line[Pindex] = NULL;
		}
		else{
			so->linenum = i;
			so->line[Pindex] = strdup(line);      /* share the line */
			i++;
			so->full[Pindex] = 1;
		}
		if(read == -1) so->EXITSIG = 1;
//--------------End of prod lock----------------------------------------------

		pthread_mutex_unlock(&so->lock);
		pthread_cond_signal(&so->cv);

		if(Pindex == 49) Pindex = 0;
		else Pindex++;

		if(read == -1) break;
	}
	free(line);
	line = NULL;
	printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
	*ret = i;
	pthread_exit(ret);
}

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	int len;
	char *line;

	while (1) {
		//grab mutex lock
		//check condvar
		pthread_mutex_lock(&so->lock);

//--------------start of cons lock------------------------------------------------------------------------
		while(so->full[so->Cindex] == 0){
			if(so->EXITSIG == 1) break;
			pthread_cond_wait(&so->cv, &so ->lock);
		}
		if(so->EXITSIG == 1 && so->full[so->Cindex] == 0){
			pthread_mutex_unlock(&so->lock);
			pthread_cond_signal(&so->cv);
			break;
		}

		line = so->line[so->Cindex];
		if (line != NULL) {
			len = strlen(line);
			printf("Cons_%x: [%02d:%02d] %s",
				(unsigned int)pthread_self(), i, so->linenum, line);
			free(so->line[so->Cindex]);
			i++;
			so->full[so->Cindex] = 0;
		}
		//release lock
		//signal condvar

                if(so->Cindex == 49) so->Cindex = 0;
                else so->Cindex++;		
//--------------End of cons lock------------------------------------------------------------------------------------

		pthread_mutex_unlock(&so->lock);
		pthread_cond_signal(&so->cv);
		if(line == NULL) break;
	}
	printf("Cons: %d lines\n", i);
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
        for(int j = 0; j<50; j++){
                share->line[j] = NULL;
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

