#include <stdio.h>
#include <stdlib.h>

typedef struct User{
   int socketClient;
   char login[50];
   struct User *suiv;
}User;

int cmdHandler(User *clients, User *sender, char *command, char args[2][256]);

void help(char *cmd);

int login(char *buffer, char *usrName);

void mp(User *client, User *target, char *msg);

void mg(User *clients, User *client, char *msg);