#include <stdio.h>
#include <stdlib.h>

typedef struct User{
   int socketClient;
   char login[50];
   struct User *suiv;
}User;

int cmdHandler(User *clients, User *sender, char *command, char args[2][256], struct pollfd *polls, int *nbrePolls);

int version(User *clients, User *sender, struct pollfd *polls, int *nbrePolls);

void help(char *cmd);

void disconnect(User *clients, User *temp, struct pollfd polls[4], int *nbrePolls);

int login(char *buffer, char *usrName);

void mp(User *clients, User *sender, User *target, char *msg, struct pollfd *polls, int *nbrePolls);

void mg(User *clients, User *client, char *msg, struct pollfd *polls, int *nbrePolls);

void serverMsg(User *clients, User *target, char *msg, struct pollfd *polls, int *nbrePolls);