int cmdHandler(User *clients, User *sender, char *command, char args[2][256], struct pollfd *polls, int *nbrePolls){
    User *temp = clients;
    char *listCommands[6];
    int i = 0;

    listCommands[0] = "/help";
    listCommands[1] = "/quit";
    listCommands[2] = "/login";
    listCommands[3] = "/mp";
    listCommands[4] = "/mg";
    listCommands[5] = "/users";

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
            disconnect(clients, sender, polls, nbrePolls);
            break;
        case 2:
            if(!strcmp(sender->login, "")){
                if(!login(sender->login, args[0])){
                    printf("Invalid Username, try again!\n");
                }else{
                    printf("Le client %s vient de se connecter!\n", sender->login);
                    strcpy(args[0], "Le client ");
                    strcat(args[0], sender->login);
                    strcat(args[0], " vient de nous rejoindre!\n");
                    mg(clients, sender, args[0], polls, nbrePolls);
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
    }

    return 1;
}

void help(char *cmd){
    system("cat command.txt");
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

    if(*nbrePolls == 2){
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
    *nbrePolls--;
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
            ecrits = write(sender->socketClient, "Le destinataire est déconnecté, le message n'a pas pu etre envoyé \n\n", strlen("Le destinataire est déconnecté, le message n'a pas pu etre envoyé \n\n"));
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
            ecrits = write(sender->socketClient, "Le message a été envoyé\n\n", strlen("Le message a été envoyé\n\n"));
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