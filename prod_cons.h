#include <pthread.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE	256
struct stat_t {
	int stat [MAX_STRING_LENGTH];
	int stat2 [ASCII_SIZE][ASCII_SIZE];
};


struct so_entry {
	int length;
	unsigned char *data;
	pthread_mutex_t lock;
	pthread_cond_t cv;
	int full;
};

#define BUFFER_SIZE		0x1000
#define BUFF_NRENTRY	0x100
typedef struct sharedobject {
	struct so_entry buff[BUFF_NRENTRY];
	struct stat_t result;
	pthread_mutex_t c_lock;
	int count;
	int c_index;
	FILE *rfile;
	int fin;
	pthread_mutex_t fin_lock;
} so_t;

unsigned char * data_buffer[BUFF_NRENTRY];

struct stat_t * do_stat(char *buffer, int length);
void print_result(struct stat_t *s);
