int find_network_newline(char *message, int inbuf) {
    for (int i = 0; i < inbuf; i++) {
        if (*(message + i) == '\n') return i;
    }
    return -1;
}

int buffer_message(char *message, int *inbuf, int *espace, char *after) {
    int bytes_read = read(STDIN_FILENO, after, 256 - *inbuf); // on récupère la taille de la ligne en nombre d'octets*
    *inbuf += bytes_read;
    int where; // où est la fin de la ligne
    // On trouve la fin de la ligne
    where = find_network_newline(message, *inbuf);
    if (where >= 0) { // On a bien une ligne
        // MAJ de inbuf et on enlève la ligne entière du buffer du client
        memmove(message, message + where + 1, *inbuf - (where + 1));
        *inbuf -= (where + 1);
    }
    // MAJ de espace et after pour préparer le prochain read
    *espace = sizeof(message) - *inbuf;
    after = message + *inbuf;

    return (where >= 0); //Si on a bien une ligne, ça retourne 1 sinon 0
}

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

void cmdHandler(char *message, char prevCmd[], char **prevArgs) {
    char *listCommands[6];
    int size = sizeof(listCommands) / sizeof(listCommands[0]);
    int returnCode;
    int i;

    listCommands[0] = "/ret";
    listCommands[1] = "/users";
    listCommands[2] = "/mp";
    listCommands[3] = "/mg";
    listCommands[4] = "/greating";
    listCommands[5] = "/login";

    char *cmds = strtok(message, "\n"); //On sépare s'il y a plusieurs commandes dans une seule chaine de caractère

    // On traite chaque commande une à la fois
    while (cmds != NULL) {
        char *Cmd = getCmd(cmds); //On récupère la commande
        char **args = getArgs(cmds, Cmd); //On récupère le/les argument(s)

        // Reconnait quelle commande a été saisie
        for (i = 0; i < size; i++) {
            if (!strcmp(listCommands[i], Cmd)) {
                break;
            }
        }

        // Traite la commande
        switch (i) {
            case 0:
                returnCode = atoi(args[0]);
                switch (returnCode) {
                    case 200:
                        if (!strcmp(prevCmd, "/mp")) {
                            printf("\x1b[1F"); // On va a la ligne précédente du terminal
                            printf("\x1b[2K"); // On efface la ligne
                            printf("\033[33mPRIVATE\033[32m You to %s\033[0m>\033[93m %s\033[0m\n", prevArgs[0],
                                   prevArgs[1]);
                        } else if (!strcmp(prevCmd, "/login")) {
                            printf("\x1b[1F"); // On va a la ligne précédente du terminal
                            printf("\x1b[2K"); // On efface la ligne
                            printf("\033[92mVotre nom d'utilisateur est maintenant\033[96m '%s'\033[0m\n\n",
                                   prevArgs[0]);
                        }
                        break;
                    case 400:
                        printf("\x1b[1F"); // On va a la ligne précédente du terminal
                        printf("\x1b[2K"); // On efface la ligne
                        printf("\033[31mBad request: Invalid command syntax '%s ", prevCmd);
                        if (!strcmp(prevCmd, "/login")) {
                            printf("$USERNAME$'\033[0m\n");
                        } else if (!strcmp(prevCmd, "/mp")) {
                            printf("$ADRESSEE$ $MESSAGE$'\033[0m\n");
                        } else if (!strcmp(prevCmd, "/mg")) {
                            printf("$MESSAGE$'\033[0m\n");
                        }
                        break;
                    case 404:
                        printf("\x1b[1F"); // On va a la ligne précédente du terminal
                        printf("\x1b[2K"); // On efface la ligne
                        printf("\033[31mNot Found: User '%s' not found\033[0m\n", prevArgs[0]);
                        break;
                    case 409:
                        printf("\x1b[1F"); // On va a la ligne précédente du terminal
                        printf("\x1b[2K"); // On efface la ligne
                        printf("\033[31mConflict: You already have a username or '%s' was taken\033[0m\n", prevArgs[0]);
                        break;
                    case 426:
                        printf("\033[31mUpdate required: Your version is too old\033[0m\n");
                        exit(-1);
                        break;
                    case 501:
                        printf("\x1b[1F"); // On va a la ligne précédente du terminal
                        printf("\x1b[2K"); // On efface la ligne
                        printf("\033[31mNot implemented: Command '%s' does not exist\033[0m\n", prevCmd);
                        break;
                }

                break;
            case 1:
                printf("\x1b[1F"); // On va a la ligne précédente du terminal
                printf("\x1b[2K"); // On efface la ligne
                char *username = strtok(args[0], " "); //On sépare les noms d'utilisateurs
                printf("\033[95mUtilisateurs: \033[96m-%s\n", username);
                username = strtok(NULL, " "); //On passe au nom suivant
                while (username != NULL) {
                    printf("              -%s\n", username); // On affiche le nom

                    username = strtok(NULL, " "); //On passe au nom suivant
                }
                printf("\033[0m\n");
                break;
            case 2:
                printf("\033[33mPRIVATE\033[32m %s to You\033[0m>\033[93m %s\033[0m\n", args[0], args[1]);
                break;
            case 3:
                printf("\033[94mGLOBAL\033[32m %s\033[0m>\033[93m %s\033[0m\n", args[0], args[1]);
                break;
            case 4:
                printf("\033[92m\n%s\033[0m\n\n", args[0]);
                break;
            case 5:
                printf("\033[93mVeuillez saisir un nom d'utilisateur\033[0m\n\n");
                break;
        }

        free(Cmd);
        free(args);
        cmds = strtok(NULL, "\n"); //On passe à la commande suivante
    }
}