#include "mutex.h"

void print(){
    int i = 0;
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
}
