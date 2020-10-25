#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MAX_STRING_LENGTH 30
#define ASCII_SIZE	256
int stat [MAX_STRING_LENGTH];
int stat2 [ASCII_SIZE];
unsigned char buffer [0x1000];

void char_stat(char *argv);

typedef struct sharedobject {
        FILE *rfile;
        int linenum;
        char *buffer[100];
        pthread_mutex_t lock;          
	pthread_cond_t producerCV;
        pthread_cond_t consumerCV[100];      
	int full[100];
        int buffersize;                  // # of buffer which is same with # of consumer
        int count;                   	// number of buffer which is filled
        int pIndex;                     // buffer index for producer
        int cIndex;                    // buffer index for consumer
        int producerFinish;
	int consumerFinish;
} so_t;

void *producer(void *arg) {
        so_t *so = arg;
        int *ret = malloc(sizeof(int));
        FILE *rfile = so->rfile;
        int i = 0;
        char *line = NULL;
        size_t len = 0;
        ssize_t read = 0;
        int in = 0;


        while (1) {
		    read = getdelim(&line, &len, '\n', rfile);

                    // check mutex lock
                    pthread_mutex_lock(&so->lock);
		
		    in = so->pIndex;

		    // check condition variable
                    while( so->count == so->buffersize) {
                        pthread_cond_wait(&so->producerCV, &so->lock);
                    }

                    // enter CS
                    if (read == -1) {
                            so->buffer[in] = NULL;
			    so->full[in] = 1;
			    so->producerFinish = 1;
        		    
			    so->pIndex = (so->pIndex + 1) % so->buffersize;
			    so->count++;

			    pthread_mutex_unlock(&so->lock);
			    pthread_cond_signal(&so->consumerCV[in]);
			    break;
	            }
                    else {
                            so->buffer[in] = strdup(line);
                            so->full[in] = 1;
			    so->pIndex = (so->pIndex + 1) % so->buffersize;
                            so->linenum = i;
                            so->count++;
                            i++;                    

	                    pthread_mutex_unlock(&so->lock);
                   	    pthread_cond_signal(&so->consumerCV[in]);
		    }

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
        int out = 0;

        while (1) {
            pthread_mutex_lock(&so->lock);

	    out = so->cIndex;

            while(so->full[out] == 0 && so->producerFinish == 0) {
		pthread_cond_wait(&so->consumerCV[out], &so->lock);
            }

            // Enter CS
            out = so->cIndex;

	    line = so->buffer[out];
            if (line != NULL) {
                    len = strlen(line);
                    printf("Cons_%x: [%02d:%02d] %s",
                    (unsigned int)pthread_self(), i, so->linenum, line);
		    char_stat(line);
                    free(line);
                    i++;
                    so->cIndex = (so->cIndex + 1) % so->buffersize;
                    so->count--;
                    so->full[out] = 0;

                    pthread_mutex_unlock(&so->lock);
                    pthread_cond_signal(&so->producerCV);
	    }
           
	    if(line == NULL) {
		so->consumerFinish++;
		pthread_mutex_unlock(&so->lock);

		for(int i = 0; i < so->buffersize; i++) {
			pthread_cond_signal(&so->consumerCV[i]);
                }
		 break;
            }

	}
                printf("Cons: %d lines\n", i);
                *ret = i;
                pthread_exit(ret);
        }


void char_stat(char* argv) {

	char *cptr = NULL;
	char *substr = NULL;
	char *brka = NULL;
	char *sep = "{}()[],;\" \n\t^";
	size_t length = 0;


	cptr = argv;
	length = strlen(cptr);

	for(int i = 0; i < length; i++){
		if(*cptr < 256 && *cptr > 1) {
			stat2[*cptr]++;			// counting for how many times each character called
		}
		cptr++;					// except for first charctecr of string
	}

	cptr = argv;

	for(substr = strtok_r(cptr, sep, &brka); substr; substr = strtok_r(NULL, sep, &brka)) {
		length = strlen(substr);
		if(length >= 30) length = 30;
		stat[length - 1]++;

		cptr = cptr + length + 1;		// setting to next word
		if(*cptr == '\0') break;		// when word is finished
	}
}





int main (int argc, char *argv[])
{
        pthread_t prod[100];
        pthread_t cons[100];
        int Nprod, Ncons;
        int rc;
        long t;
        int *ret;
        int i;
        FILE *rfile;

	// 
	int sum = 0;
	memset(stat, 0, sizeof(stat));
	memset(stat2, 0, sizeof(stat2));
	//

        if (argc == 1) {
                printf("usage: ./prod_cons <readfile> #Producer #Consumer\n");
                exit (0);
        }
        so_t *share = malloc(sizeof(so_t));
        memset(share, 0, sizeof(so_t));

        // size init--------------
        share->buffersize = atoi(argv[3]);
        //---------------------

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

            for(int i = 0; i < atoi(argv[2]); i++){
                share->buffer[i] = NULL;
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




	// sum
	sum = 0;
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
	
   printf("A & a : %8d\n", stat2['A']+stat2['a']);
   printf("B & b : %8d\n", stat2['B']+stat2['b']);           
   printf("C & c : %8d\n", stat2['C']+stat2['c']);
   printf("D & d : %8d\n", stat2['D']+stat2['d']);
   printf("E & e : %8d\n", stat2['E']+stat2['e']);
   printf("F & f : %8d\n", stat2['F']+stat2['f']);
   printf("G & g : %8d\n", stat2['G']+stat2['g']);
   printf("H & h : %8d\n", stat2['H']+stat2['h']);
   printf("I & i : %8d\n", stat2['I']+stat2['i']);
   printf("J & j : %8d\n", stat2['J']+stat2['j']);
   printf("K & k : %8d\n", stat2['K']+stat2['k']);
   printf("L & l : %8d\n", stat2['L']+stat2['l']);
   printf("M & m : %8d\n", stat2['M']+stat2['m']);
   printf("N & n : %8d\n", stat2['N']+stat2['n']);
   printf("O & o : %8d\n", stat2['O']+stat2['o']);
   printf("P & p : %8d\n", stat2['P']+stat2['p']);
   printf("Q & q : %8d\n", stat2['Q']+stat2['q']);
   printf("R & r : %8d\n", stat2['R']+stat2['r']);
   printf("S & s : %8d\n", stat2['S']+stat2['s']);
   printf("T & t : %8d\n", stat2['T']+stat2['t']);
   printf("U & u : %8d\n", stat2['U']+stat2['u']);
   printf("V & v : %8d\n", stat2['V']+stat2['v']);
   printf("W & w : %8d\n", stat2['W']+stat2['w']);
   printf("X & x : %8d\n", stat2['X']+stat2['x']);
   printf("Y & y : %8d\n", stat2['Y']+stat2['y']);
   printf("Z & z : %8d\n", stat2['Z']+stat2['z']);


            pthread_exit(NULL);
            exit(0);
    }


