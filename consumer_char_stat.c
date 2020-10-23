#include "mutex.h"

void* consumer_char_stat(void* arg) {
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
        else
            so->signal = 1;
        //release lock
        //signal condvar
        if (so->cidx == 9)
            so->cidx = 0;
        else
            so->cidx++;
        pthread_mutex_unlock(&so->lock);
        pthread_cond_signal(&so->cv);
    }
    printf("Cons_%x: %d lines\n", (unsigned int)pthread_self(), i);
    *ret = i;
    pthread_exit(ret);
}
