#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_STRING_LENGTH 30
#define ASCII_SIZE	256
int stat [MAX_STRING_LENGTH];
int stat2 [ASCII_SIZE][ASCII_SIZE];
unsigned char buffer [0x1000];
size_t length = 0;
size_t length2 = 0;
int ii = 0;
int line_num = 1;
int sum = 0;
int global_iter = 0;
int freadRC = 0;

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
		if (read == -1) so->line[Pindex] = NULL;
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
	int exit = 0;
	int stat_add_signal = 0;
	int stat2_add_signal = 0;

	while (1) {

                char *cptr = NULL;
                char *substr = NULL;
                char *brka = NULL;
                char *sep = "{}()[],;\" \n\t^";

                pthread_mutex_lock(&so->lock);

//--------------start of cons lock------------------------------------------------------------------------
		while(so->full[so->Cindex] == 0){
			if(so->EXITSIG == 1) break;
			pthread_cond_wait(&so->cv, &so->lock);
		}
		if(so->EXITSIG == 1 && so->full[so->Cindex] == 0){
                        pthread_mutex_unlock(&so->lock);
                        pthread_cond_signal(&so->cv);
                        break;
                }

		//Enter CS
		line = so->line[so->Cindex];
		if (line != NULL) {
			len = strlen(line);
			printf("Cons_%x: [%02d:%02d] %s",
				(unsigned int)pthread_self(), i, so->linenum, line);
		cptr = line;
		for (substr = strtok_r(cptr, sep, &brka);
                        substr;
                        substr = strtok_r(NULL, sep, &brka))
		{
                        length = strlen(substr);
			length2 = length;
			if (length >= 30) {length2 = length; length = 30;}
			stat[length-1]++;
			if (*cptr == '\n') break;
                        cptr = substr;
                        for (int i = 0 ; i < length2 + 1 ; i++) {
                                if (*cptr < 256 && *cptr > 1) {
                                        stat2[*cptr][0]++;
                                }
                                cptr++;
                        }
		}
		
			free(so->line[so->Cindex]);
			i++;
			so->full[so->Cindex] = 0;
		}
		else {
			so->EXITSIG = 1;
		}
		
		if(so->Cindex == 49) so->Cindex = 0;
		else so->Cindex++;
		
//--------------End of cons lock------------------------------------------------------------------------------------

                pthread_mutex_unlock(&so->lock);
                pthread_cond_signal(&so->cv);

		if(line == NULL){
			break;
		}
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
	for(int j = 0; j<10; j++){
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

	for (i = 0 ; i < 30 ; i++) {
                sum += stat[i];
        }

        // print out distributions
        printf("*** print out distributions *** \n");
        printf("  #ch  freq \n");
        for (i = 0 ; i < 30 ; i++) {
                int j = 0;
                int num_star = stat[i]*80/sum;
                printf("[%3d]: %4d \t", i+1, stat[i]);
                for (j = 0 ; j < num_star ; j++)
                        printf("*");
                printf("\n");
        }
        printf("       A        B        C        D        E        F        G        H        I        J        K        L        M        N        O        P        Q        R        S        T        U        V        W        X        Y        Z\n");
        printf("%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
                        stat2['A'][0]+stat2['a'][0], stat2['B'][0]+stat2['b'][0],  stat2['C'][0]+stat2['c'][0],  stat2['D'][0]+stat2['d'][0],  stat2['E'][0]+stat2['e'][0],
                        stat2['F'][0]+stat2['f'][0], stat2['G'][0]+stat2['g'][0],  stat2['H'][0]+stat2['h'][0],  stat2['I'][0]+stat2['i'][0],  stat2['J'][0]+stat2['j'][0],
                        stat2['K'][0]+stat2['k'][0], stat2['L'][0]+stat2['l'][0],  stat2['M'][0]+stat2['m'][0],  stat2['N'][0]+stat2['n'][0],  stat2['O'][0]+stat2['o'][0],
                        stat2['P'][0]+stat2['p'][0], stat2['Q'][0]+stat2['q'][0],  stat2['R'][0]+stat2['r'][0],  stat2['S'][0]+stat2['s'][0],  stat2['T'][0]+stat2['t'][0],
                        stat2['U'][0]+stat2['u'][0], stat2['V'][0]+stat2['v'][0],  stat2['W'][0]+stat2['w'][0],  stat2['X'][0]+stat2['x'][0],  stat2['Y'][0]+stat2['y'][0],
                        stat2['Z'][0]+stat2['z'][0]);
	
	pthread_exit(NULL);
	exit(0);
}
