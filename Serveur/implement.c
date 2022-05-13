User* cmdHandler(User *clients, User *sender, char *command, char args[2][256], struct pollfd *polls, int *nbrePolls){
    User *temp = clients;
    char *listCommands[6];
    int i = 0;

    listCommands[0] = "/quit";
    listCommands[1] = "/login";
    listCommands[2] = "/mp";
    listCommands[3] = "/mg";
    listCommands[4] = "/users";
    listCommands[5] = "/version";

    for(i = 0 ; i < 6 ; i++){
        if(!strcmp(listCommands[i], command)){
            break;
        }
    }

    switch(i){
        case 0:
            printf("Le client %s s'est déconnecté.\n", sender->login);
            strcpy(args[0], "/mg serveur Le client ");
            strcat(args[0], sender->login);
            strcat(args[0], " s'est déconnecté.\n");

            mg(clients, sender->login, args[0], polls, nbrePolls);

            clients = disconnect(clients, sender->login, polls, nbrePolls);

            break;
        case 1:
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

                        mg(clients, sender->login, args[0], polls, nbrePolls);
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
        case 2:
            while(temp != NULL && strcmp(temp->login, args[0]) != 0){
                temp = temp->suiv;
            }
            if(temp == NULL){
                serverMsg(clients, sender, "/ret 404", polls, nbrePolls);
                break;
            }else if(!strcmp(args[1], "")){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                break;
            }else if(strlen(args[1]) > 1024){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                break;
            }else if(!strcmp(args[0], sender->login)){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
                break;
            }
            char msgpriv[1049] = {"/mp "};
            strcat(msgpriv, sender->login);
            strcat(msgpriv, " ");
            strcat(msgpriv, args[1]);
            if(mp(clients, temp, msgpriv)){
                serverMsg(clients, sender, "/ret 200", polls, nbrePolls);
            }else{
                clients = disconnect(clients, temp->login, polls, nbrePolls);
                serverMsg(clients, sender, "/ret 404", polls, nbrePolls);
            }

            break;
        case 3:
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
            mg(clients, sender->login, msgglob, polls, nbrePolls);

            break;
        case 4:
            users(clients, sender, polls, nbrePolls);

            break;
        case 5:
            if(version(args[0])){
                serverMsg(clients, sender, "/ret 200", polls, nbrePolls);
                serverMsg(clients, sender, "/greeting", polls, nbrePolls);
                serverMsg(clients, sender, "/login", polls, nbrePolls);
            }else if(!version(args[0])){
                serverMsg(clients, sender, "/ret 400", polls, nbrePolls);
            }else{
                serverMsg(clients, sender, "/ret 426", polls, nbrePolls);
            }

            break;
    }

    return clients;
}

int version(char *msg){
    if(!strcmp(msg, "") || !strcmp(msg, " ")){
        return 0;
    }else if(strcmp(msg, "0.1b") != 0 && strcmp(msg, "0.1c") != 0){
        return 2;
    }
    return 1;
}

User* disconnect(User *clients, char *Lequel, struct pollfd *polls, int *nbrePolls){
    if(clients == NULL){
        return NULL;
    }else if(clients->suiv == NULL){
        free(clients);
        polls[1].fd = polls[1].events = polls[1].revents = 0;
        return NULL;
    }

    User *temp = clients;

    while(temp != NULL){
        if(!strcmp(temp->login, Lequel)){
            close(temp->socketClient);
            for(int i = 1 ; i < *nbrePolls ; i++){
                if(polls[i].fd == temp->socketClient){
                    polls[i].revents = 0;
                    for(int j = i ; j < 3 ; j++){
                        polls[j].fd = polls[j + 1].fd;
                        polls[j].events = polls[j + 1].events;
                    }
                    polls[3].fd = polls[3].events = 0;
                    break;
                }
            }
            --*nbrePolls;

            if(temp->socketClient == clients->socketClient){
                clients = clients->suiv;
                free(temp);
            }else{
                User* delete = temp;
                temp = clients;

                while(temp->suiv != NULL && temp->suiv->socketClient != delete->socketClient) temp = temp->suiv;

                temp->suiv = temp->suiv->suiv;
                free(delete);
            }

            break;
        }
        temp = temp->suiv;
    }

    return clients;
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

int mp(User *clients, User *target, char *msg){
    int ecrits = write(target->socketClient, msg, strlen(msg));
    switch(ecrits){
        case -1 :
            perror("write");
            close(target->socketClient);
            exit(-3);
        case 0 :
            fprintf(stderr, "La client %s a fermé la socket !\n\n", target->login);
            return 0;
        default:
            return 1;
    }
}

void mg(User *clients, char *senderName, char *msg, struct pollfd *polls, int *nbrePolls){
    User *temp = clients;

    while(temp != NULL){
        if(strcmp(temp->login, senderName) != 0 && mp(clients, temp, msg) == 0) clients = disconnect(clients, temp->login, polls, nbrePolls);

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
            clients = disconnect(clients, target->login, polls, nbrePolls);
            break;
    }
}