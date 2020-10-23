#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE	256
int stat[MAX_STRING_LENGTH];
int stat2[ASCII_SIZE];
unsigned char buffer[0x1000];

typedef struct sharedobject {
	FILE* rfile;
	int linenum;
	char* line[10];
	pthread_mutex_t lock; //for mutual exclusion
	pthread_cond_t cv; //condvar for checking full
	int full[10];
	int cidx;
	int signal;
} so_t;

void* producer(void* arg);
void* consumer_prod_cons(void* arg);
void* consumer_char_stat(void* arg);
void print();
