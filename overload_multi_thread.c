#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ASCII_TABLE_SIZE 256

int total_buff_num;
int consumer_cnt=0;//consumer match buff count
int dic[ASCII_TABLE_SIZE];//final char count array
void show();//print char count 

pthread_mutex_t give_acc_num=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tmp_grab = PTHREAD_MUTEX_INITIALIZER;

typedef struct sharedobject {
	FILE *rfile;
	char *line[50];
	pthread_mutex_t lock[50];
	int get[50];
	int finish;
} so_t;

void *producer(void *arg) {
	int num=0;
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i = 0;
	int su=0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;

	while (1) {
		//find empty buff
//		pthread_mutex_lock(&tmp_grab);
/*		while(so->get[num]!=0){
			num+=1;
			num=num%total_buff_num;
			if(so->get[num]==0){pthread_mutex_lock(&so->lock[num]);break;}
		}*/
//		pthread_mutex_unlock(&tmp_grab);
		if(so->get[num] != 0){num++;num=num%total_buff_num;continue;}
//		pthread_mutex_unlock(&tmp_grab);
		//else{pthread_mutex_lock(&so->lock[num]);}
	//	if(so->get[num != 0]){pthread_mutex_unlock(&so->lock[num]);continue;}
		read = getdelim(&line , &len, '\n',rfile);


		if(read == -1){//만약 마지막 줄이라면
			for(int nu=0;nu<total_buff_num;nu++){
				if(so->get[nu]==0){
					so->get[nu]=1;
					so->line[nu]=NULL;
					pthread_mutex_unlock(&so->lock[nu]);
				}
				else{so->finish=1;}//끝이다고 표기 
			}
			break;
		}



		so->line[num] = strdup(line);//복사해서넣고
		so->get[num] = 1;//넣었다고 체크
		i+=1;

		pthread_mutex_unlock(&so->lock[num]);
	}
	free(line);
	printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
	*ret = i;
	pthread_exit(ret);
}

void *consumer(void *arg) {
	
	int *mini_dic = malloc(sizeof(int)*257);//line_count 257 index
	memset(mini_dic,0,sizeof(mini_dic));

	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	int su=0;
	int len=0;
	char *line;
	int buff_ord;

/////////////////////////////////////////
	pthread_mutex_lock(&give_acc_num);
	buff_ord = consumer_cnt;
	consumer_cnt+=1;
	pthread_mutex_unlock(&give_acc_num);
/////////////////////////////////////////

	while(1){
		
		if(so->finish==1 && so->get[buff_ord]==0){break;}//end

		while(so->get[buff_ord]==0);//wait

		pthread_mutex_lock(&so->lock[buff_ord]);
		line = so->line[buff_ord];

		if(line==NULL){
				////////////here?
			pthread_mutex_unlock(&so->lock[buff_ord]);
			break;
		}
		len = strlen(line);
		//////////////////////////////////////
		//일부러 넣은 부하
		for(int f=0;f<10000;f++){
			su+=f;
			su=0;
		}
		for(int ch=0;ch<len;ch++){
			if(*line<256 && *line>=1){mini_dic[*line]+=1;}
			line+=1;
		}
		free(so->line[buff_ord]);
		mini_dic[257]+=1;//line_count
		so->get[buff_ord]=0;
		pthread_mutex_unlock(&so->lock[buff_ord]);
	}
	printf("consumer %d: %d lines\n",buff_ord,mini_dic[257]);//line	
	pthread_exit(mini_dic);
}


int main (int argc, char *argv[])
{	time_t start,end;	
	pthread_t prod[50];
	pthread_t cons[50];
	memset(dic,0,sizeof(dic));
	int *get_mini_dic = malloc(sizeof(int)*256);
	memset(get_mini_dic,0,sizeof(get_mini_dic));
	int Nprod, Ncons;
	int rc;   long t;
	int *ret;
	int i;
	printf("////////////////////////////////////////\n\n32154231 정영환 \n\n멀티스레트 프로그래밍 과제\n\n\n추가구현\n\nmultistep_pipline ->cd python_prod_cons\n\n\npython3 one_step_network.py\n\n\npython3 multi_step_network.py\n\n///////////////////////////////////////////\n\n\n ***주의 : 위 코드는 성능측정과 가설검증을 위해서 소비자에  부하가 걸려있습니다.\n소비자에 반복문을 주석처리 하면 됩니다. \n\nlet's start\n\n ");
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
		if (Nprod > 50) {printf("sorry over 50 , resize 50\n");Nprod = 50;}//////////////
		if (Nprod == 0) Nprod = 1;
	} else Nprod = 1;
	if (argv[3] != NULL) {
		Ncons = atoi(argv[3]);
		if (Ncons > 50) {printf("sorry over 50 , resize 50 \n");Ncons = 50;}//////////
		if (Ncons == 0) Ncons = 1;
	} else Ncons = 1;
	
	start = time(NULL);
	total_buff_num = Ncons;
	share->rfile = rfile;

	for (i = 0 ; i < Nprod ; i++)
		pthread_create(&prod[i], NULL, producer, share);
	for (i = 0 ; i < Ncons ; i++)
		pthread_create(&cons[i], NULL, consumer, share);

	printf("main continuing\n");//start

	for (i = 0 ; i < Ncons ; i++) {
		rc = pthread_join(cons[i], (void **) &get_mini_dic);
		for(int ch=0;ch<256;ch++){dic[ch]+=get_mini_dic[ch];}
		printf("main: consumer_%d dealing with %d line\n", i, get_mini_dic[257]);
	}
	for (i = 0 ; i < Nprod ; i++) {
		rc = pthread_join(prod[i], (void **) &ret);
		printf("main: producer_%d dealing with %d line\n", i, *ret);
	}
	show();
	end = time(NULL);
	printf("total_time : %f\n",(double)(end-start));
	pthread_exit(NULL);
	exit(0);
}


void show(){
	for(int i=33;i<123;i++){
		printf("char %c count : %d\n",i,dic[i]);
	}


}

