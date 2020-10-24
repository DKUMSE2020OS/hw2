/*	Mobile Sys. Eng. OS
	HW2: Multi-threaded word count
	32161570 Kitae Park	*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE        256
#define BUFF_NRENTRY	0x100

int stat [MAX_STRING_LENGTH];
int stat2 [ASCII_SIZE];

void *producer(void *arg);
void *consumer(void *arg);
void do_stat(char *arg);
void print_result();

typedef struct sharedobject {
	FILE *rfile;
	int linenum;
	pthread_mutex_t lock;
	pthread_cond_t cv;

	char *data_buffer[BUFF_NRENTRY];//Char_Buffer	
	int full[BUFF_NRENTRY];		//Status of Buffer
	int count;			//Prod_Count
	int c_index;			//Cons_Count
	int fin; 			//Consumer's Read Status
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

	while(1){
		pthread_mutex_lock(&so->lock);	//Grab mutex lock
		//Calculate Prod_Count
		in = so->count;
		(so->count)++;
		so->count %= BUFF_NRENTRY;

		while(so->full[in] == 1){
			pthread_cond_wait(&so->cv, &so->lock);	//check Cond_var
		}
	
		//Enter Critical Section	
		read = getdelim(&line, &len, '\n', rfile);

		if(read == -1){		//Read Failed
			so->full[in] = 1;
			so->data_buffer[in] = NULL;
		} else {
			so->full[in] = 1;
			so->data_buffer[in] = strdup(line);	//Share the Line
			so->linenum = i++;
		}

		pthread_mutex_unlock(&so->lock);	//Unlock
		pthread_cond_signal(&so->cv);		//Send Signal
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
	char *line = NULL;
	
	int out = 0;

	while (1) {
		pthread_mutex_lock(&so->lock);	//Grab Mutex Lock
		//Calculate Cons_Count
		out = so->c_index;
		(so->c_index)++;
		so->c_index %= BUFF_NRENTRY;

		while(so->full[out] == 0){	//Buffer is not full
			if(so->fin == 1)	//Cons Read Done
				break;
			pthread_cond_wait(&so->cv, &so->lock);	//check Cond_var
		}

		if (so->fin == 1) {	//Cons Read Done
			pthread_mutex_unlock(&so->lock);	//Grab Mutex Lock
			pthread_cond_signal(&so->cv);		//Check Cond_Var
			break;
		}


		line = so->data_buffer[out];	//Read from Buffer

		if(line != NULL){	//Read Success
			//printf("Cons_%x: [%02d:%02d] %s", (unsigned int)pthread_self(), i, so->linenum, line);
    		do_stat(line);		//Analyze statistics of String	
			free(so->data_buffer[out]);
			i++;
			so->full[out] = 0;
		} else {
			so->fin = 1;	//Read Finished
		}

		pthread_mutex_unlock(&so->lock);	//Unlock
		pthread_cond_signal(&so->cv);		//Send Signal
	}

	//printf("Cons: %d lines\n", i);
	*ret = i;
	pthread_exit(ret);
}

void do_stat(char *arg){

	char *cptr = NULL;
    char *word = NULL;
    char *brka = NULL;
    char *sep = "{}()[],;\" \n\t^";
    size_t length, length2 = 0;
    
    cptr = arg;

    for (word = strtok_r(cptr, sep, &brka); 
    	word;
    	word = strtok_r(NULL, sep, &brka))
    {
        if (*cptr == '\0')
            break;
        length = strlen(word);
        length2 = length;
        if (length >= 30) {
            length = 30;
        }
        stat[length - 1]++;

        cptr = word;
        for (int i = 0; i < length2 + 1; i++) {
            if (*cptr < 256 && *cptr > 1) {
                stat2[*cptr]++;
            }
            cptr++;
        }
    }

    return;
}

void print_result(){

	int sum = 0;
	for (int i = 0; i < 30; i++) {
    	sum += stat[i];
	}
	printf("\nSUM = %d\n", sum);
	printf("***** print out statistics of Lines ***** \n");
	for (int i = 0; i < 30; i++){
    	
    	int num_star = stat[i]*80/sum;

     	printf("[%2d]: %4d \t", i+1, stat[i]);
    	
    	for(int j = 0; j < num_star; j++)
    		printf("*");
    	printf("\n");
   }
	printf("*****print out statistics of Alphabets***** \n");
	printf("A(a) : %8d\n", stat2['A']+stat2['a']);
	printf("B(b) : %8d\n", stat2['B']+stat2['b']);           
	printf("C(c) : %8d\n", stat2['C']+stat2['c']);
	printf("D(d) : %8d\n", stat2['D']+stat2['d']);
	printf("E(e) : %8d\n", stat2['E']+stat2['e']);
	printf("F(f) : %8d\n", stat2['F']+stat2['f']);
	printf("G(g) : %8d\n", stat2['G']+stat2['g']);
	printf("H(h) : %8d\n", stat2['H']+stat2['h']);
	printf("I(i) : %8d\n", stat2['I']+stat2['i']);
	printf("J(j) : %8d\n", stat2['J']+stat2['j']);
	printf("K(k) : %8d\n", stat2['K']+stat2['k']);
	printf("L(l) : %8d\n", stat2['L']+stat2['l']);
	printf("M(m) : %8d\n", stat2['M']+stat2['m']);
	printf("N(n) : %8d\n", stat2['N']+stat2['n']);
	printf("O(o) : %8d\n", stat2['O']+stat2['o']);
	printf("P(p) : %8d\n", stat2['P']+stat2['p']);
	printf("Q(q) : %8d\n", stat2['Q']+stat2['q']);
	printf("R(r) : %8d\n", stat2['R']+stat2['r']);
	printf("S(s) : %8d\n", stat2['S']+stat2['s']);
	printf("T(t) : %8d\n", stat2['T']+stat2['t']);
	printf("U(u) : %8d\n", stat2['U']+stat2['u']);
	printf("V(v) : %8d\n", stat2['V']+stat2['v']);
	printf("W(w) : %8d\n", stat2['W']+stat2['w']);
	printf("X(x) : %8d\n", stat2['X']+stat2['x']);
	printf("Y(y) : %8d\n", stat2['Y']+stat2['y']);
	printf("Z(z) : %8d\n", stat2['Z']+stat2['z']);
	printf("------------------Program End------------------\n");
}

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

	rfile = fopen((char*)argv[1], "r");	//Open file(argv[1])
	if (rfile == NULL) {
		perror("rfile");
		exit(0);
	}
	if (argv[2] != NULL) {	//Get #of Prod.
		Nprod = atoi(argv[2]);
		if (Nprod > 100) Nprod = 100;
		if (Nprod == 0) Nprod = 1;
	}
	else Nprod = 1;
	if (argv[3] != NULL) {	//Get #of Prod.
		Ncons = atoi(argv[3]);
		if (Ncons > 100) Ncons = 100;
		if (Ncons == 0) Ncons = 1;
	}
	else Ncons = 1;

	printf("------------------Multi-threaded word count------------------\n");
	printf("File Name: %s\n#of Producers: %d\n#of Consumers: %d\n\n", (char*)argv[1], Nprod, Ncons);

	share->rfile = rfile;
	memset(stat, 0, sizeof(stat));
	memset(stat2, 0, sizeof(stat2));

	pthread_cond_init(&share->cv, NULL);
	pthread_mutex_init(&share->lock, NULL);

	for (i = 0; i < Nprod; i++)
		pthread_create(&prod[i], NULL, producer, share);
	for (i = 0; i < Ncons; i++)
		pthread_create(&cons[i], NULL, consumer, share);
	printf("main continuing\n");

	for (i = 0; i < Ncons; i++) {
		rc = pthread_join(cons[i], (void**)&ret);
		printf("main: consumer_%d joined with %d\n", i, *ret);
	}
	for (i = 0; i < Nprod; i++) {
		rc = pthread_join(prod[i], (void**)&ret);
		printf("main: producer_%d joined with %d\n", i, *ret);
	}

	print_result();
	pthread_exit(NULL);
	exit(0);
}
