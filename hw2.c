#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#define FULL 0x100
#define ASCII_SIZE 256

sem_t full;
sem_t empty;

char *b_line[FULL];


typedef struct sharedobject {
	FILE *rfile;
	int linenum;
	int index;
	int n_cons;
	int filled;
	int stat[ASCII_SIZE];
	int stat2[300];
	pthread_mutex_t lock;
	pthread_mutex_t s_lock;
} so_t;

void *producer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int n_cons = so->n_cons;
	int i = 0;
	int filled=0;
	int linenum =0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;

	while (1) { 
		read = getdelim(&line, &len, '\n', rfile);
		if (read == -1) {			
			for(int i=0;i<n_cons;i++){                
				sem_post(&full);} 
			break;}
		else{
			sem_wait(&empty);
			if(linenum ==FULL) filled++;
			linenum = linenum % FULL;
			b_line[linenum] = strdup(line);
			linenum++;	
			sem_post(&full);
			i++;
		}
	}
	so->filled =filled;
	free(line);
	printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
	*ret = i;
	pthread_exit(ret);
}

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int linenum=0;
	int index=0;
	int length=0;
	int i=0;
	char *line;
	int L_stat[ASCII_SIZE] = {0};
	int L_stat2[100]={0};

	char *cptr=NULL;
	char *substr = NULL;
	char *brka = NULL;
	char *sep = "{}()[],;\" \n\t";
	int length2;

	while (1) {
		sem_wait(&full);
		//mutex_lock
		pthread_mutex_lock(&so->lock);

		index = so->index;
		linenum = (so->linenum) % FULL;
		(so->linenum)++;
		(so->index)++;
		line =b_line[linenum];
		b_line[linenum]=NULL;

		pthread_mutex_unlock(&so->lock);
		//release lock
		sem_post(&empty);
		i++;
		if(line==NULL) break;

		//cpu working
		cptr =line;
		for (substr = strtok_r(cptr, sep, &brka);
				substr;
				substr = strtok_r(NULL, sep, &brka))
		{
			length = strlen(substr);

		//	L_stat2[length-1]++;

			if (*cptr == '\0') break;

			cptr=substr;	
			for(int i=0; i<length+1; i++){
				if(*cptr<256 && *cptr>1){
					L_stat[*cptr]++;}
				cptr++;}
		}
		//cpu working

	}
	//get s_lock
	pthread_mutex_lock(&so->s_lock);
	for(int i=0;i<ASCII_SIZE;i++){
		so->stat[i] = so->stat[i]+ L_stat[i];
	}
	for(int i=0; i<100;i++){
		so->stat2[i] = so->stat2[i] + L_stat2[i];
	}
	pthread_mutex_unlock(&so->s_lock);
	//release s_lock
	free(line);
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
//	memset(stat2,0,sizeof(stat2));

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
	share->n_cons = Ncons;
	pthread_mutex_init(&share->lock, NULL);
	pthread_mutex_init(&share->s_lock,NULL);
	sem_init(&full,0,0);
	sem_init(&empty,0,FULL);


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
		printf("Buffer was filled %d times in total\n",share->filled);
	}
		
	printf("*** print out distributions ***\n");

	printf("       A        B        C        D        E        F        G        H        I        J        K        L        M        N        O        P        Q        R        S        T        U        V        W        X        Y        Z\n");
	printf("%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
			share->stat['A']+share->stat['a'], share->stat['B']+share->stat['b'],  share->stat['C']+share->stat['c'],  share->stat['D']+share->stat['d'],  share->stat['E']+share->stat['e'],
			share->stat['F']+share->stat['f'], share->stat['G']+share->stat['g'],  share->stat['H']+share->stat['h'],  share->stat['I']+share->stat['i'],  share->stat['J']+share->stat['j'],
			share->stat['K']+share->stat['k'], share->stat['L']+share->stat['l'],  share->stat['M']+share->stat['m'],  share->stat['N']+share->stat['n'],  share->stat['O']+share->stat['o'],
			share->stat['P']+share->stat['p'], share->stat['Q']+share->stat['q'],  share->stat['R']+share->stat['r'],  share->stat['S']+share->stat['s'],  share->stat['T']+share->stat['t'],
			share->stat['U']+share->stat['u'], share->stat['V']+share->stat['v'],  share->stat['W']+share->stat['w'],  share->stat['X']+share->stat['x'],  share->stat['Y']+share->stat['y'],
			share->stat['Z']+share->stat['z']);


	pthread_exit(NULL);
	free(share);
	free(rfile);
	exit(0);
}

