clean : serveur
	rm serveur.o

serveur : serveur.o
	gcc -o serveur serveur.o

serveur.o : serveur.c header.h implement.c
	gcc -c -o serveur.o serveur.c