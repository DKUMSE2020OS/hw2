#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "prod_cons.h"

struct stat_t * do_stat(char *buffer, int length)
{
	struct stat_t* s = malloc(sizeof(struct stat_t));
	
	// initialize stat
	memset(s, 0, sizeof(struct stat_t));

	char *line = buffer;
	char *cptr = NULL;
	char *substr = NULL;
	char *brka = NULL;
	char *sep = "{}()[],;\" \n\t";
	int rc = 0;
	int length2;

	for (int len = 0 ; len < length; len++) {
		cptr = &line[len];
		if (*cptr < 256 && *cptr > 1) {
				s->stat2[*cptr][0]++;
				s->stat2[*cptr][*(cptr+1)]++;
#ifdef _IO_
				printf("# of %c(%d): %d\n", *cptr, *cptr, s->stat2[*cptr][0]);
#endif
		}
	}

	cptr = line;
#ifdef _IO_
	printf("[%3d] %s\n", line_num++, line);
#endif
	for (substr = strtok_r(cptr, sep, &brka);
		substr;
		substr = strtok_r(NULL, sep, &brka))
	{
		length = strlen(substr);
		length2 = length;
		// update stats
#ifdef _IO_
		printf("length: %d\n", (int)length);
#endif
		if (length >= 30) length2= 30;
		s->stat[length2-1]++;
	}

	return s;
}

void print_result(struct stat_t *s)
{
	// sum
	int sum = 0;
	for (int i = 0 ; i < 30 ; i++) {
		sum += s->stat[i];
	}
	if (sum != 0) {
	// print out distributions
	printf("*** print out distributions *** \n");
	printf("  #ch  freq \n");
	for (int i = 0 ; i < 30 ; i++) {
		int j = 0;
		int num_star = s->stat[i]*80/sum;
		printf("[%3d]: %4d \t", i+1, s->stat[i]);
		for (j = 0 ; j < num_star ; j++)
			printf("*");
		printf("\n");
	}
	}
	printf("       A        B        C        D        E        F        G        H        I        J        K        L        M        N        O        P        Q        R        S        T        U        V        W        X        Y        Z\n");
	printf("%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
			s->stat2['A'][0]+s->stat2['a'][0], s->stat2['B'][0]+s->stat2['b'][0],  s->stat2['C'][0]+s->stat2['c'][0],  s->stat2['D'][0]+s->stat2['d'][0],  s->stat2['E'][0]+s->stat2['e'][0],
			s->stat2['F'][0]+s->stat2['f'][0], s->stat2['G'][0]+s->stat2['g'][0],  s->stat2['H'][0]+s->stat2['h'][0],  s->stat2['I'][0]+s->stat2['i'][0],  s->stat2['J'][0]+s->stat2['j'][0],
			s->stat2['K'][0]+s->stat2['k'][0], s->stat2['L'][0]+s->stat2['l'][0],  s->stat2['M'][0]+s->stat2['m'][0],  s->stat2['N'][0]+s->stat2['n'][0],  s->stat2['O'][0]+s->stat2['o'][0],
			s->stat2['P'][0]+s->stat2['p'][0], s->stat2['Q'][0]+s->stat2['q'][0],  s->stat2['R'][0]+s->stat2['r'][0],  s->stat2['S'][0]+s->stat2['s'][0],  s->stat2['T'][0]+s->stat2['t'][0],
			s->stat2['U'][0]+s->stat2['u'][0], s->stat2['V'][0]+s->stat2['v'][0],  s->stat2['W'][0]+s->stat2['w'][0],  s->stat2['X'][0]+s->stat2['x'][0],  s->stat2['Y'][0]+s->stat2['y'][0],
			s->stat2['Z'][0]+s->stat2['z'][0]);

	return ;
}
