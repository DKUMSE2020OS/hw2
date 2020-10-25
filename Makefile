all : 1prod_1cons multiple_cons multiple_cons_stat multiple_cons_stat2

CC = gcc

LDFLAGS+= -lpthread

1prod_1cons: 1prod_1cons.c
	$(CC) $^ $(LDFLAGS) -o $@

multiple_cons: multiple_cons.c
	$(CC) $^ $(LDFLAGS) -o $@

multiple_cons_stat: multiple_cons_stat.c
	$(CC) $^ $(LDFLAGS) -o $@

multiple_cons_stat2: multiple_cons_stat2.c
	$(CC) $^ $(LDFLAGS) -o $@


