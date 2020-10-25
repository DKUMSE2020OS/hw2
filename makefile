all : prod_cons char_stat

prod_cons : prod_cons.o producer.o consumer_prod_cons.o
	gcc -o prod_cons prod_cons.o producer.o consumer_prod_cons.o -lpthread

char_stat : char_stat.o producer.o consumer_char_stat.o print.o
	gcc -o char_stat char_stat.o producer.o consumer_char_stat.o print.o -lpthread

prod_cons.o : prod_cons.c
	gcc -c -o prod_cons.o prod_cons.c

char_stat.o : char_stat.c
	gcc -c -o char_stat.o char_stat.c

producer.o : producer.c
	gcc -c -o producer.o producer.c

consumer_prod_cons.o : consumer_prod_cons.c
	gcc -c -o consumer_prod_cons.o consumer_prod_cons.c

consumer_char_stat.o : consumer_char_stat.c
	gcc -c -o consumer_char_stat.o consumer_char_stat.c

print.o : print.c
	gcc -c -o print.o print.c

clean :
	rm *.o prod_cons char_stat

