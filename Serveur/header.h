#include <stdio.h>
#include <stdlib.h>

typedef struct User{
    int socketClient;
    char login[50];
    struct User *suiv;
}User;

User* cmdHandler(User *clients, User *sender, char *command, char args[2][256], struct pollfd *polls, int *nbrePolls, char greeting[100]);

int version(char *msg);

User* disconnect(User *clients, char *Lequel, struct pollfd polls[4], int *nbrePolls);

int login(char *buffer, char *usrName);

int mp(User *clients, User *target, char *msg);

void mg(User *clients, char *senderName, char *msg, struct pollfd *polls, int *nbrePolls);

void users(User *clients, User *sender, struct pollfd *polls, int *nbrePolls);

void serverMsg(User *clients, User *target, char *msg, struct pollfd *polls, int *nbrePolls);