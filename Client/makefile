run : client clean
	./client -p 5000

clean :
	rm client.o

client : client.o
	gcc -o client client.o

client.o : client.c header.h implement.c
	gcc -c -o client.o client.c