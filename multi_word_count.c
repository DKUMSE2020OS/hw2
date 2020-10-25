// Operating System HW2-3 multi_word_count.c 
// 32163122 Minseo Lee (Github: minseodaily)
// Copyright 2020 All Rights Reserved by minseodaily

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE 256
int stat [MAX_STRING_LENGTH];
int stat2 [ASCII_SIZE][ASCII_SIZE];
void analysis(char *cptr2);

typedef struct sharedobject {
	FILE *rfile;
	char *line[100];
	pthread_mutex_t lock;
	pthread_cond_t cv;
	int full[100];
	int cindex;
	int signal;
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
		read = getdelim(&line, &len, '\n', rfile);

		pthread_mutex_lock(&so->lock);

		while (so->full[pindex] == 1) {
			pthread_cond_wait(&so->cv, &so->lock);
		}

		if (read == -1) {
			so->line[pindex] = NULL;
		} else {
			so->line[pindex] = strdup(line);
			i++;
			so->full[pindex] = 1;
		}

		pthread_mutex_unlock(&so->lock);
		pthread_cond_broadcast(&so->cv);
		
		if (pindex == 99) {
			pindex = 0;
		} else {
			pindex++;
		}

		if (read == -1) {
			so->signal = 1;
			break;
		}
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
	char *cptr1 = NULL;

	while (1) {
		pthread_mutex_lock(&so->lock);

		while (so->full[so->cindex] == 0){
			if(so->signal == 1){
				break;
			}
			pthread_cond_wait(&so->cv, &so->lock);
		}

		if (so->full[so->cindex] == 0 && so->signal == 1 ) {
			pthread_mutex_unlock(&so->lock);
			pthread_cond_broadcast(&so->cv);
			break;
		}

		line = so->line[so->cindex];

		if (line != NULL) {
			cptr1 = line;
			analysis(cptr1);
			free(line);
			i++;
			so->full[so->cindex] = 0;
		} else {
			so->signal = 1;
			break;
		}

		if(so->cindex == 99) {
			so->cindex = 0;
		} else {
			so->cindex++;
		}

		pthread_mutex_unlock(&so->lock);
		pthread_cond_broadcast(&so->cv);
	}

	*ret = i;
	pthread_exit(ret);
}

void analysis(char *cptr2) {
	size_t length1 = 0;
	size_t length2 = 0;
	char *substr = NULL;
      	char *brka = NULL;
      	char *sep = "{}()[],;\" \n\t^";

	for (substr = strtok_r(cptr2, sep, &brka); substr; substr = strtok_r(NULL, sep, &brka))
	{   
         	length1 = strlen(substr);
		length2 = length1;

		if (length1 >= 30) { 
			length1 = 30;
		}

		stat[length1-1]++;
		if (*cptr2 == '\n') break;

		cptr2 = substr;
         	for (int i = 0 ; i < length2 + 1 ; i++) {
            		if (*cptr2 < 256 && *cptr2 > 1) {
               			stat2[*cptr2][0]++;
			}
            	cptr2++;
		}
	}
}

void print_result(){
	int sum = 0;

	for (int i = 0 ; i < 30 ; i++) {
		sum += stat[i];
	}

	printf("\n\n--------------- print out distributions ------------------ \n");
	printf("  #ch  freq \n");
	for (int i = 0 ; i < 30 ; i++) {
		int num_star = stat[i]*80/sum;
		printf("[%3d]: %4d \t", i+1, stat[i]);
		for (int j = 0 ; j < num_star ; j++)
			printf("*");
		printf("\n");
	}

	printf("------------------------------------------------------------\n");
	printf("        A         B         C         D         E          \n");
	printf("%9d %9d %9d %9d %9d          \n", stat2['A'][0]+stat2['a'][0], stat2['B'][0]+stat2['b'][0],  stat2['C'][0]+stat2['c'][0],  stat2['D'][0]+stat2['d'][0],  stat2['E'][0]+stat2['e'][0]);
	printf("------------------------------------------------------------\n");
	printf("        F         G         H         I         J          \n");
	printf("%9d %9d %9d %9d %9d          \n", stat2['F'][0]+stat2['f'][0], stat2['G'][0]+stat2['g'][0],  stat2['H'][0]+stat2['h'][0],  stat2['I'][0]+stat2['i'][0],  stat2['J'][0]+stat2['j'][0]);
	printf("------------------------------------------------------------\n");
	printf("        K         L         M         N         O          \n");
	printf("%9d %9d %9d %9d %9d          \n", stat2['K'][0]+stat2['k'][0], stat2['L'][0]+stat2['l'][0],  stat2['M'][0]+stat2['m'][0],  stat2['N'][0]+stat2['n'][0],  stat2['O'][0]+stat2['o'][0]);
	printf("------------------------------------------------------------\n");
	printf("        P         Q         R         S         T          \n");
	printf("%9d %9d %9d %9d %9d          \n", stat2['P'][0]+stat2['p'][0], stat2['Q'][0]+stat2['q'][0],  stat2['R'][0]+stat2['r'][0],  stat2['S'][0]+stat2['s'][0],  stat2['T'][0]+stat2['t'][0]);
	printf("------------------------------------------------------------\n");
	printf("        U         V         W         X         Y         Z \n");
	printf("%9d %9d %9d %9d %9d %9d \n", stat2['U'][0]+stat2['u'][0], stat2['V'][0]+stat2['v'][0],  stat2['W'][0]+stat2['w'][0],  stat2['X'][0]+stat2['x'][0],  stat2['Y'][0]+stat2['y'][0],
			stat2['Z'][0]+stat2['z'][0]);
	printf("------------------------------------------------------------\n");
	
}

int main (int argc, char *argv[])
{
	pthread_t prod[100];
	pthread_t cons[100];
	char *line = NULL;
	int Nprod, Ncons;
	int rc;   long t;
	int *ret;
	int i;
	FILE *rfile;

	memset(stat, 0, sizeof(stat));
	memset(stat2, 0, sizeof(stat2));

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

	print_result();

	pthread_exit(NULL);
	exit(0);
}
