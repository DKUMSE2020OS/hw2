#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE	256
int stat [MAX_STRING_LENGTH];
int stat2 [ASCII_SIZE][ASCII_SIZE];
unsigned char buffer [0x1000];

int main(int argc, char *argv[])
{
	int rc = 0;
	size_t length = 0;
	int i = 0;
	FILE *rfile = NULL;
	char *line = NULL;
	int line_num = 1;
	int sum = 0;
	int global_iter = 0;

	if (argc == 1) {
		printf("usage: ./stat <filename> <iter>\n");
		exit(0);
	}
	if (argc == 3) {
		global_iter = atoi(argv[2]);
	} else global_iter = 1;

	for (int iter = 0; iter < global_iter ; iter++) {
	// Open argv[1] file
	rfile = fopen((char *) argv[1], "rb");
	if (rfile == NULL) {
		perror(argv[1]);
		exit(0);
	}
	// initialize stat
	memset(stat, 0, sizeof(stat));
	memset(stat2, 0, sizeof(stat2));

	while (1) {
		char *cptr = NULL;
		char *substr = NULL;
		char *brka = NULL;
		char *sep = "{}()[],;\" \n\t^";
		// For each line,
		//rc = getdelim(&line, &length, '\n', rfile);
		memset(buffer, 0, sizeof(buffer));
		rc = fread(buffer, sizeof(buffer), 1, rfile);
		line = buffer;
		if (rc == 0) break;

		cptr = line;
#ifdef _IO_
		printf("[%3d] %s\n", line_num++, line);
#endif
		for (substr = strtok_r(cptr, sep, &brka);
			substr;
			substr = strtok_r(NULL, sep, &brka))
		{
			length = strlen(substr);
			// update stats
#ifdef _IO_
			printf("length: %d\n", (int)length);
#endif
			if (length >= 30) length = 30;
			stat[length-1]++;
			if (*cptr == '\0') break;

			cptr = substr;
			for (int i = 0 ; i < length+1 ; i++) {
				if (*cptr < 256 && *cptr > 1) {
					stat2[*cptr][0]++;
					stat2[*cptr][*(cptr+1)]++;
#ifdef _IO_
					printf("# of %c(%d): %d\n", *cptr, *cptr, stat2[*cptr]);
#endif
				}
				cptr++;
			}
			cptr++;
		}
	}
	} // global_iter

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
	printf("       A        B        C        D        E        F        G        H        I        J        K        L        M        N        O        P        Q        R        S        T        U        V        W        X        Y        Z\n");
	printf("%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
			stat2['A'][0]+stat2['a'][0], stat2['B'][0]+stat2['b'][0],  stat2['C'][0]+stat2['c'][0],  stat2['D'][0]+stat2['d'][0],  stat2['E'][0]+stat2['e'][0],
			stat2['F'][0]+stat2['f'][0], stat2['G'][0]+stat2['g'][0],  stat2['H'][0]+stat2['h'][0],  stat2['I'][0]+stat2['i'][0],  stat2['J'][0]+stat2['j'][0],
			stat2['K'][0]+stat2['k'][0], stat2['L'][0]+stat2['l'][0],  stat2['M'][0]+stat2['m'][0],  stat2['N'][0]+stat2['n'][0],  stat2['O'][0]+stat2['o'][0],
			stat2['P'][0]+stat2['p'][0], stat2['Q'][0]+stat2['q'][0],  stat2['R'][0]+stat2['r'][0],  stat2['S'][0]+stat2['s'][0],  stat2['T'][0]+stat2['t'][0],
			stat2['U'][0]+stat2['u'][0], stat2['V'][0]+stat2['v'][0],  stat2['W'][0]+stat2['w'][0],  stat2['X'][0]+stat2['x'][0],  stat2['Y'][0]+stat2['y'][0],
			stat2['Z'][0]+stat2['z'][0]);

	//if (line != NULL) free(line);	
	// Close the file
	fclose(rfile);

	return 0;
}
