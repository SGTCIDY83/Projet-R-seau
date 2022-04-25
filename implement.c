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
            strcpy(args[0], "/mg serveur Le client ");
            strcat(args[0], sender->login);
            strcat(args[0], " s'est déconnecté.\n");
            disconnect(clients, sender, polls, nbrePolls);
            
            User *serv = malloc(sizeof(User));
            strcpy(serv->login, "Serveur");
            mg(clients, serv, args[0], polls, nbrePolls);
            free(serv);
            break;
        case 2:
            if(!strcmp(sender->login, "")){
                while(temp != NULL && strcmp(args[0], temp->login) != 0){
                    temp = temp->suiv;
                }

                if(temp == NULL){
                    if(!login(sender->login, args[0])){
                        serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                    }else{
                        printf("\nLe client %s vient de se connecter!\n", sender->login);
                        strcpy(args[0], "/mg serveur Le client ");
                        strcat(args[0], sender->login);
                        strcat(args[0], " vient de nous rejoindre!\n");
                        User *serv = malloc(sizeof(User));
                        strcpy(serv->login, "serveur");
                        mg(clients, serv, args[0], polls, nbrePolls);
                        free(serv);
                    }
                }else{
                    serverMsg(clients, sender, "/ret 409", polls, nbrePolls);
                    serverMsg(clients, sender, "/mp serveur Ce nom d'utilisateur est déjà pris\n\n", polls, nbrePolls);
                }
            }else{
                serverMsg(clients, sender, "/ret 409", polls, nbrePolls);
                serverMsg(clients, sender, "/mp serveur Vous avez déjà un nom d'utilisateur\n\n", polls, nbrePolls);
            }
            break;
        case 3:
            while(temp != NULL && strcmp(temp->login, args[0]) != 0){
                temp = temp->suiv;
            }
            if(temp == NULL){
                serverMsg(clients, sender, "/ret 404", polls, nbrePolls);
                printf("Le client %s n'éxiste pas.\n", args[0]);
                break;
            }else if(!strcmp(args[1], "")){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                printf("Pas de message.\n");
                break;
            }else if(strlen(args[1]) > 1024){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                printf("Message trop long.\n");
            }
            char msgpriv[1049] = {"/mp "};
            strcat(msgpriv, sender->login);
            strcat(msgpriv, " ");
            strcat(msgpriv, args[1]);
            mp(clients, sender, temp, msgpriv, polls, nbrePolls);

            break;
        case 4:
            if(!strcmp(args[0], "")){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                printf("Pas de message.\n");
                break;
            }else if(strlen(args[0]) > 1024){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                printf("Message trop long.\n");
                break;
            }
            char msgglob[1049] = {"/mg "};
            strcat(msgglob, sender->login);
            strcat(msgglob, " ");
            strcat(msgglob, args[0]);
            mg(clients, sender, msgglob, polls, nbrePolls);

            break;
        case 5:
            users(clients, sender, polls, nbrePolls);

            break;
        case 6:
            if(version(sender, args[0]) == 1){
                serverMsg(clients, sender, "/ret 200", polls, nbrePolls);
                serverMsg(clients, sender, "/greating", polls, nbrePolls);
                serverMsg(clients, sender, "/login", polls, nbrePolls);
            }else if(!version(sender, args[0])){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
            }else{
                serverMsg(clients, sender, "/ret 426", polls, nbrePolls);
            }

            break;
    }

    return 1;
}

int version(User *sender, char *msg){
    if(!strcmp(msg, "") || !strcmp(msg, " ")){
        return 0;
    }else if(strcmp(msg, "0.1b") != 0 && strcmp(msg, "0.1c") != 0){
        return 2;
    }
    return 1;
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
            serverMsg(clients, sender, "/ret 200", polls, nbrePolls);
   }
}

void mg(User *clients, User *sender, char *msg, struct pollfd *polls, int *nbrePolls){
    User *temp = clients;

    while(temp != NULL){
        if(strcmp(temp->login, sender->login) != 0) mp(clients, sender, temp, msg, polls, nbrePolls);

        temp = temp->suiv;
    }
}

void users(User *clients, User *sender, struct pollfd *polls, int *nbrePolls){
    char msg[1035] = "/mp serveur Utilisateurs: ";
    User *temp = clients;

    while(temp != NULL){
        strcat(msg, temp->login);
        if(temp->suiv != NULL) strcat(msg, " ; ");
        temp = temp->suiv;
    }

    serverMsg(clients, sender, msg, polls, nbrePolls);
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