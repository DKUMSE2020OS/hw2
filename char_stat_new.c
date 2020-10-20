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
    char* line[10];
    pthread_mutex_t lock; //for mutual exclusion
    pthread_cond_t cv; //condvar for checking full
    int full[10];
    int cidx;
    int signal;
} so_t;

void* producer(void* arg) {
    so_t* so = arg;
    int* ret = malloc(sizeof(int));
    FILE* rfile = so->rfile;
    int i = 0;
    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    int pidx = 0;

    while (1) {
        //grab mutex lock
        //check condvar
        pthread_mutex_lock(&so->lock);
        while (so->full[pidx] == 1) {
            pthread_cond_wait(&so->cv, &so->lock);
        }
        //enter CS
        read = getdelim(&line, &len, '\n', rfile);
        if (read == -1) {
            so->full[pidx] = 1;
            so->line[pidx] = NULL;
        }
        else {
            so->full[pidx] = 1;
            so->line[pidx] = strdup(line);      /* share the line */
            i++;
        }
        if (pidx == 9)
            pidx = 0;
        else
            pidx++;
        //release lock
        //signal condvar
        pthread_mutex_unlock(&so->lock);
        pthread_cond_signal(&so->cv);
        if (read == -1) break;
    }
    free(line);
    printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
    *ret = i;
    pthread_exit(ret);
}

void* consumer(void* arg) {
    so_t* so = arg;
    int* ret = malloc(sizeof(int));
    int i = 0;
    int length;
    int length2;
    char* line;
    char* word;
    char* cptr = NULL;
    char* brka = NULL;
    char* sep = "{}()[],;\" \n\t^";

    while (1) {
        //grab mutex lock
        //check condvar
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
        //Enter CS
        line = so->line[so->cidx];
        if (line != NULL) {
            cptr = line;
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

                //캐릭터 분석
                cptr = word;
                for (int i = 0; i < length2+1; i++) {
                    if (*cptr < 256 && *cptr > 1) {
                        stat2[*cptr]++;
                    }
                    cptr++; //다음 문자
                }
            }
            free(so->line[so->cidx]);
            i++;
            so->full[so->cidx] = 0;
        }
        else {
            so->signal = 1;
            break;
        }
        //release lock
        //signal condvar
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
        printf("usage: ./char_stat_new <readfile> #Producer #Consumer\n");
        exit(0);
    }

    so_t* share = malloc(sizeof(so_t));
    memset(share, 0, sizeof(so_t));
    rfile = fopen((char*)argv[1], "r");
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
    int sum = 0;
    int sum2 = 0;
    for (i = 0; i < 30; i++) {
        sum += stat[i];
    }
    for (i = 65; i < 91; i++) {
        sum2 += stat2[i];
    }
    for (i = 97; i < 123; i++) {
        sum2 += stat2[i];
    }
    // print out distributions
    printf("\n--------Alphabet Statistics----------------------print out distributions----------------\n");
    printf(" ABCs\t  #ABC  \tratio \t\t #ch\t  freq  \tratio\t  graph\n");
    printf("[Tot]:\t%10d     100.00%%\t\t[Tot]:\t%9d      100.00%%\n", sum2,sum);
    for (i = 0; i < 30; i++) {
        int k = stat2[i + 65] + stat2[i + 97];
        int num_star = stat[i] * 80 / sum;
        if (i < 26)
            printf("[ %c ]:\t %9d\t%5.2f%%\t\t[%3d]:   %8d\t%5.2f%%\t",
                i + 65, k, (double)k * 100 / sum2, i + 1, stat[i], (double)stat[i] * 100 / sum);
        else
            printf("\t\t\t\t\t[%3d]:  %9d\t%5.2f%%\t", i + 1, stat[i], (double)stat[i] * 100 / sum);
        for (int j = 0; j < num_star; j++)
            printf("*");
        printf("\n");
    }
    printf("----------------------------------------------------------------------------------------\n");
    pthread_exit(NULL);
    exit(0);
}
