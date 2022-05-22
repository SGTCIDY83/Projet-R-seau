clean : client
	rm client.o

client : client.o
	gcc -o client client.o

client.o : client.c header.h implement.c
	gcc -c -o client.o client.c