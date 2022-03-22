int cmdHandler(User *clients, User *sender, char *command, char args[2][256]){
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
                //help(args[0]);
                break;
            case 1:
                //quit();
                break;
            case 2:
                if(!strcmp(sender->login, "")){
                    if(!login(sender->login, args[0])){
                        printf("Invalid Username, try again!\n");
                    }else{
                        printf("Bonjour %s!\n", sender->login);
                        //Envoyer une annonce à tout le monde de son arrivée
                    }
                }else{
                    printf("You are already logged in as %s!\n", sender->login);
                }
                break;
            case 3:
                User *temp2 = clients;
                while(temp2 != NULL && strcmp(temp2->login, args[0]) != 0){
                    temp2 = temp2->suiv;
                }
                if(temp2 == NULL){
                    printf("Le client %s n'éxiste pas.\n", args[0]);
                    break;
                }else if(!strcmp(args[1], "")){
                    printf("Vous n'avez pas écrit de message.\n");
                    break;
                }
                mp(sender, args[0], args[1]);

                break;
    }

    return 1;
}
//void help(char *cmd){}

int login(char *buffer, char *usrName){
    for(int i = 0 ; i < strlen(usrName) ; i++){
        if(usrName[i] < 45 || (usrName[i] > 46 && usrName[i] < 48) || (usrName[i] > 57 && usrName[i] < 65) || (usrName[i] > 90 && usrName[i] < 95) || (usrName[i] > 95 && usrName[i] < 97) || usrName[i] > 122){
            return 0;
        }
    }

    strcpy(buffer, usrName);

    return 1;
}

void mp(User *sender, User *target, char *msg){
    //sender envoie msg à target

    int ecrits = write(target->socketClient, msg, strlen(msg));
    switch(ecrits){
        case -1 :
            perror("write");
            close(target->socketClient);
            exit(-3);
        case 0 :
            fprintf(stderr, "La client %s a fermé la socket !\n\n");

            int ecrits = write(sender->socketClient, "Le destinataire est déconnecté, le message n'a pas pu etre envoyé \n\n", strlen("Le destinataire est déconnecté, le message n'a pas pu etre envoyé \n\n"));

            switch(ecrits){
                case -1 :
                    perror("write");
                    close(sender->socketClient);
                    exit(-3);
                case 0:
                  fprintf(stderr, "La client %s a fermé la socket !\n\n");
                  close(sender->socketClient);  
                  // disconnet()
                    break;
            }

            
            //disconnect()
            close(target->socketClient);
            break;
        default:

            int ecrits = write(sender->socketClient, "Le message a été envoyé\n\n", strlen("Le message a été envoyé\n\n"));

            switch(ecrits){
                case -1 :
                    perror("write");
                    close(sender->socketClient);
                    exit(-3);
                case 0:
                  fprintf(stderr, "La client %s a fermé la socket !\n\n");
                  close(sender->socketClient);  
                  // disconnet()
                    break;
            }
            //renvoyer un code de succès à sender
   }
}

void mg(User *clients, User *sender, char *msg){

   User *temp = clients;

   while(temp != NULL){
        mp(sender, temp, msg);
        temp = temp->suiv;
   }
}