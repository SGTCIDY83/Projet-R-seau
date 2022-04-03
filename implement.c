int cmdHandler(User *clients, User *sender, char *command, char args[2][256], struct pollfd *polls, int *nbrePolls){
    User *temp = clients;
    char *listCommands[7];
    int i = 0;

    listCommands[0] = "/help";
    listCommands[1] = "/quit";
    listCommands[2] = "/login";
    listCommands[3] = "/mp";
    listCommands[4] = "/mg";
    listCommands[5] = "/users";
    listCommands[6] = "/version";

    for(i = 0 ; i < 6 ; i++){
        if(!strcmp(listCommands[i], command)){
            break;
        }
    }

    switch(i){
        case 0:
            help(args[0]);
            break;
        case 1:
            printf("Le client %s s'est déconnecté.\n", sender->login);
            strcpy(args[0], "Le client ");
            strcat(args[0], sender->login);
            strcat(args[0], " s'est déconnecté.\n");
            User *serv = NULL;
            strcpy(serv->login, "Serveur");
            mg(clients, serv, args[0], polls, nbrePolls);
            free(serv);
            disconnect(clients, sender, polls, nbrePolls);
            break;
        case 2:
            if(!strcmp(sender->login, "")){
                while(temp != NULL && strcmp(args[0], temp->login) != 0){
                    temp = temp->suiv;
                }

                if(temp == NULL){
                    if(!login(sender->login, args[0])){
                        serverMsg(clients, sender, "Nom d'utilisateur incorrect\n\n", polls, nbrePolls);
                    }else{
                        printf("\nLe client %s vient de se connecter!\n", sender->login);
                        strcpy(args[0], "/mp serveur Le client ");
                        strcat(args[0], sender->login);
                        strcat(args[0], " vient de nous rejoindre!\n");
                        User *serv = malloc(sizeof(User));
                        strcpy(serv->login, "serveur");
                        mg(clients, serv, args[0], polls, nbrePolls);
                        free(serv);
                    }
                }else{
                    serverMsg(clients, sender, "/mp serveur Ce nom d'utilisateur est déjà pris\n\n", polls, nbrePolls);
                }
            }else{
                serverMsg(clients, sender, "Vous êtes déjà connecté...\n\n", polls, nbrePolls);
            }
            break;
        case 3:
            while(temp != NULL && strcmp(temp->login, args[0]) != 0){
                temp = temp->suiv;
            }
            if(temp == NULL){
                printf("Le client %s n'éxiste pas.\n", args[0]);
                break;
            }else if(!strcmp(args[1], "")){
                printf("Vous n'avez pas écrit de message.\n");
                break;
            }
            mp(clients, sender, temp, args[1], polls, nbrePolls);

            break;
        case 6:
            if(!version(clients, sender, polls, nbrePolls)){
                disconnect(clients, sender, polls, nbrePolls);
            }
    }

    return 1;
}

int version(User *clients, User *sender, struct pollfd *polls, int *nbrePolls){
    int ecrits = write(sender->socketClient, "/mp serveur 0.1b", strlen("/mp serveur v0.1b"));
    switch(ecrits){
        case -1 :
            perror("write");
            close(sender->socketClient);
            exit(-3);
        case 0 :
            fprintf(stderr, "La client a fermé la socket !\n\n");
            return 0;
        default:
            ecrits = write(sender->socketClient, "/mp serveur Bienvenu!\nVeuillez vous identifier avec la commande '/login $username$'\n", strlen("/mp serveur Bienvenu!\nVeuillez vous identifier avec la commande '/login $username$'\n"));
            switch(ecrits){
                case -1 :
                    perror("write");
                    close(sender->socketClient);
                    exit(-3);
                case 0 :
                    fprintf(stderr, "La client a fermé la socket !\n\n");
                    return 0;
                default:
                    return 1;
            }
    }
}

void help(char *cmd){
    system("cat command.txt");
    printf("\n\n");
}

void disconnect(User *clients, User *temp, struct pollfd *polls, int *nbrePolls){
    close(temp->socketClient);
    for(int i = 1 ; i < *nbrePolls ; i++){
        if(polls[i].fd == temp->socketClient){
            polls[i].revents = 0;
            for(int j = i ; j < 3 ; j++){
                polls[j].fd = polls[j + 1].fd;
                polls[j].events = polls[j + 1].events;
            }
            polls[3].fd = 0;
            polls[3].events = 0;
            break;
        }
    }
    --*nbrePolls;

    if(*nbrePolls == 1){
        clients = NULL;
        free(temp);
    }else{
        User *temp2 = temp;
        temp = clients;
        while(temp->suiv != temp2 && temp->suiv != NULL){
            temp = temp->suiv;
        }
        temp->suiv = temp2->suiv;
        free(temp2);
    }
}

int login(char *buffer, char *usrName){
    for(int i = 0 ; i < strlen(usrName) ; i++){
        if(usrName[i] < 45 || (usrName[i] > 46 && usrName[i] < 48) || (usrName[i] > 57 && usrName[i] < 65) || (usrName[i] > 90 && usrName[i] < 95) || (usrName[i] > 95 && usrName[i] < 97) || usrName[i] > 122){
            return 0;
        }
    }

    strcpy(buffer, usrName);

    return 1;
}

void mp(User *clients, User *sender, User *target, char *msg, struct pollfd *polls, int *nbrePolls){
    int ecrits = write(target->socketClient, msg, strlen(msg));
    switch(ecrits){
        case -1 :
            perror("write");
            close(target->socketClient);
            exit(-3);
        case 0 :
            fprintf(stderr, "La client %s a fermé la socket !\n\n", target->login);
            ecrits = write(sender->socketClient, "/ret 001\n\n", strlen("/ret 001\n\n"));
            switch(ecrits){
                case -1 :
                    perror("write");
                    close(sender->socketClient);
                    exit(-3);
                case 0:
                    fprintf(stderr, "La client %s a fermé la socket !\n\n", sender->login);
                    disconnect(clients, sender, polls, nbrePolls);
                    break;
            }
            disconnect(clients, target, polls, nbrePolls);
            break;
        default:
            ecrits = write(sender->socketClient, "/ret 200\n\n", strlen("/ret 200\n\n"));
            switch(ecrits){
                case -1 :
                    perror("write");
                    close(sender->socketClient);
                    exit(-3);
                case 0:
                    fprintf(stderr, "La client %s a fermé la socket !\n\n", sender->login);
                    disconnect(clients, sender, polls, nbrePolls);
                    break;
            }
   }
}

void mg(User *clients, User *sender, char *msg, struct pollfd *polls, int *nbrePolls){
   User *temp = clients;

   while(temp != NULL){
        mp(clients, sender, temp, msg, polls, nbrePolls);
        temp = temp->suiv;
   }
}

void serverMsg(User *clients, User *target, char *msg, struct pollfd *polls, int *nbrePolls){
    int ecrits = write(target->socketClient, msg, strlen(msg));
    switch(ecrits){
        case -1 :
            perror("write");
            close(target->socketClient);
            exit(-3);
        case 0:
            fprintf(stderr, "La client %s a fermé la socket !\n\n", target->login);
            disconnect(clients, target, polls, nbrePolls);
            break;
    }
}