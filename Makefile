run : serveur clean
	Serveur/serveur -p 5000

clean :
	rm Serveur/*.o

serveur : Serveur/serveur.o
	gcc -o Serveur/serveur Serveur/serveur.o

serveur.o : Serveur/serveur.c Serveur/header.h Serveur/implement.c
	gcc -c -o Serveur/serveur.o Serveur/serveur.c

client.o : Client/client.c Client/header.h Client/implement.c
	gcc -c -o client/client.o client/client.c