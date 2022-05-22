char *getCmd(char message[]) {
    char *cmd = malloc(LG_MESSAGE * sizeof(char));
    for (int i = 0; i < strlen(message); i++) {
        if (message[i] != 32 && message[i] != 10) {
            cmd[i] = message[i];
        } else {
            break;
        }
    }

    return cmd;
}

char **getArgs(char message[], char cmd[]) {
    char **args = malloc(2 * sizeof(char *));
    for (int i = 0; i < 2; i++) {
        args[i] = malloc(LG_MESSAGE * sizeof(char));
    }
    int j = 0;
    int sub = strlen(cmd) + 1;
    for (int i = strlen(cmd) + 1; i < strlen(message); i++) {
        if ((!strcmp(cmd, "/mp") || !strcmp(cmd, "/mg")) && j == 0) {
            if (message[i] != 32) {
                args[j][i - sub] = message[i];
            } else {
                j++;
                sub = i + 1;
            }
            continue;
        } else if (message[i] != 10) {
            args[j][i - sub] = message[i];
        }
    }

    return args;
}

User *cmdHandler(User *clients, User *sender, char *message, struct pollfd *polls, int *nbrePolls,
                 char greeting[100]) {
    User *temp = clients;
    char *listCommands[5];
    int size = sizeof(listCommands) / sizeof(listCommands[0]);
    char *command = getCmd(message);
    char **args = getArgs(message, command);
    int i = 0;

    listCommands[0] = "/login";
    listCommands[1] = "/mp";
    listCommands[2] = "/mg";
    listCommands[3] = "/users";
    listCommands[4] = "/version";

    for (i = 0; i < size; i++) {
        if (!strcmp(listCommands[i], command)) {
            break;
        }
    }

    switch (i) {
        case 0:
            for (int i = 0; i < strlen(args[0]); i++) {
                args[1][i] = args[0][i];
            }

            if (strcmp(sender->login, "") != 0) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            } else if (!strcmp(args[1], "you")) {
                serverMsg(clients, sender, "/ret 409\n", polls, nbrePolls);
                break;
            }

            while (temp != NULL && strcmp(args[0], temp->login) != 0) {
                temp = temp->suiv;
            }

            if (temp != NULL) {
                serverMsg(clients, sender, "/ret 409\n", polls, nbrePolls);
                break;
            }

            if (!login(sender->login, args[0])) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            }

            serverMsg(clients, sender, "/ret 200\n", polls, nbrePolls);
            strcpy(args[0], "/mg serveur Le client ");
            strcat(args[0], sender->login);
            strcat(args[0], " vient de nous rejoindre!\n");

            mg(clients, sender->login, args[0], polls, nbrePolls);
            break;
        case 1:
            while (temp != NULL && strcmp(temp->login, args[0]) != 0) {
                temp = temp->suiv;
            }
            if (temp == NULL) {
                serverMsg(clients, sender, "/ret 404\n", polls, nbrePolls);
                break;
            } else if (!strcmp(args[1], "")) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            } else if (strlen(args[1]) > 1024) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            } else if (!strcmp(args[0], sender->login)) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            }
            char msgpriv[1049] = {"/mp "};
            strcat(msgpriv, sender->login);
            strcat(msgpriv, " ");
            strcat(msgpriv, args[1]);
            if (mp(clients, temp, msgpriv)) {
                serverMsg(clients, sender, "/ret 200\n", polls, nbrePolls);
            } else {
                disconnect(&clients, temp->login, polls, nbrePolls);
                serverMsg(clients, sender, "/ret 404\n", polls, nbrePolls);
            }

            break;
        case 2:
            if (!strcmp(args[0], "")) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            } else if (strlen(args[0]) > 1024) {
                serverMsg(clients, sender, "/ret 400\n", polls, nbrePolls);
                break;
            }
            char msgglob[1049] = {"/mg "};
            strcat(msgglob, sender->login);
            strcat(msgglob, " ");
            strcat(msgglob, args[0]);
            mg(clients, sender->login, msgglob, polls, nbrePolls);

            break;
        case 3:
            users(clients, sender, polls, nbrePolls);

            break;
        case 4:
            if (version(args[0]) == 1) {
                char greetingMess[110] = "/greating ";
                strcat(greetingMess, greeting);
                serverMsg(clients, sender, "/ret 200\n", polls, nbrePolls);
                serverMsg(clients, sender, greetingMess, polls, nbrePolls);
                serverMsg(clients, sender, "/login\n", polls, nbrePolls);
            } else if (version(args[0]) == 0) {
                serverMsg(clients, sender, "/ret 426\n", polls, nbrePolls);
            }

            break;
        case 5:
            serverMsg(clients, sender, "/ret 501\n", polls, nbrePolls);
            break;
    }

    return clients;
}

int version(char *msg) {
    if (strcmp(msg, "0.1b") != 0 && strcmp(msg, "0.1c") != 0) {
        return 0;
    }
    return 1;
}

void disconnect(User **clients, char *Lequel, struct pollfd *polls, int *nbrePolls) {
    User *temp = *clients;

    User *prev = NULL;

    if (temp != NULL && !strcmp(temp->login, Lequel)) {
        close(temp->socketClient);
        for (int i = 1; i < *nbrePolls; i++) {
            if (polls[i].fd == temp->socketClient) {
                polls[i].revents = 0;
                for (int j = i; j < 3; j++) {
                    polls[j].fd = polls[j + 1].fd;
                    polls[j].events = polls[j + 1].events;
                }
                polls[3].fd = polls[3].events = 0;
                break;
            }
        }
        --*nbrePolls;

        *clients = temp->suiv;
        free(temp);
        return;
    }
    while (temp != NULL && strcmp(temp->login, Lequel) != 0) {
        prev = temp;
        temp = temp->suiv;
    }

    if (temp == NULL) return;

    close(temp->socketClient);
    for (int i = 1; i < *nbrePolls; i++) {
        if (polls[i].fd == temp->socketClient) {
            polls[i].revents = 0;
            for (int j = i; j < 3; j++) {
                polls[j].fd = polls[j + 1].fd;
                polls[j].events = polls[j + 1].events;
            }
            polls[3].fd = polls[3].events = 0;
            break;
        }
    }
    --*nbrePolls;

    prev->suiv = temp->suiv;

    free(temp);
}

int login(char *buffer, char *usrName) {
    for (int i = 0; i < strlen(usrName); i++) {
        if (usrName[i] < 45 || (usrName[i] > 46 && usrName[i] < 48) || (usrName[i] > 57 && usrName[i] < 65) ||
            (usrName[i] > 90 && usrName[i] < 95) || (usrName[i] > 95 && usrName[i] < 97) || usrName[i] > 122) {
            return 0;
        }
    }

    strcpy(buffer, usrName);

    return 1;
}

int mp(User *clients, User *target, char *msg) {
    int ecrits = write(target->socketClient, msg, strlen(msg));
    switch (ecrits) {
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

void mg(User *clients, char *senderName, char *msg, struct pollfd *polls, int *nbrePolls) {
    User *temp = clients;

    while (temp != NULL) {
        if (strcmp(temp->login, senderName) != 0 && mp(clients, temp, msg) == 0)
            disconnect(&clients, temp->login, polls, nbrePolls);

        temp = temp->suiv;
    }
}

void users(User *clients, User *sender, struct pollfd *polls, int *nbrePolls) {
    char msg[1035] = "/users ";
    User *temp = clients;

    while (temp != NULL) {
        strcat(msg, temp->login);
        if (temp->suiv != NULL) strcat(msg, " ");
        temp = temp->suiv;
    }

    serverMsg(clients, sender, msg, polls, nbrePolls);
}

void serverMsg(User *clients, User *target, char *msg, struct pollfd *polls, int *nbrePolls) {
    int ecrits = write(target->socketClient, msg, strlen(msg));
    switch (ecrits) {
        case -1 :
            perror("write");
            close(target->socketClient);
            exit(-3);
        case 0:
            fprintf(stderr, "La client %s a fermé la socket !\n\n", target->login);
            disconnect(&clients, target->login, polls, nbrePolls);
            break;
    }
}