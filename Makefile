over_load_multithread.exe : overload_multi_thread.o
	gcc -o over_load_multithread.exe overload_multi_thread.o -pthread

overload_multi_thread.o : overload_multi_thread.c
	gcc -c -o overload_multi_thread.o overload_multi_thread.c -pthread

clean :
	rm *.o over_load_multithread.exe
