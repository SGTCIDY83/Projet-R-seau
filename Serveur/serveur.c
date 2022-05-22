#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>
#include "./header.h"
#include "./implement.c"

int main(int argc, char **argv) {
    int PORT = -1;
    char serverName[20] = "We Chat";
    char greeting[100] = "Bonjour et bienvenue sur le serveur We Chat!\n";
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-p")) {
            if (i == argc - 1) {
                printf("Error : Missing port number after -p argument\n");
                exit(-6);
            } else if (!(PORT = atoi(argv[i + 1]))) {
                if (!strcmp(argv[i + 1], "-g") || !strcmp(argv[i + 1], "-n")) {
                    printf("Error : Missing port number after -p argument\n");
                    exit(-6);
                }
                printf("Error : argument -p must be followed by an integer type of argument\n");
                exit(-6);
            }
        } else if (!strcmp(argv[i], "-g")) {
            if (i == argc - 1 || !strcmp(argv[i + 1], "-p") || !strcmp(argv[i + 1], "-n")) {
                printf("Error : Missing greeting message after -g argument\n");
                exit(-6);
            }
            strcpy(greeting, argv[i + 1]);
            strcat(greeting, "\n");
            if (i == argc - 1 && PORT == -1) {
                printf("Error : argument -p is missing\n");
                exit(-6);
            }
        } else if (!strcmp(argv[i], "-n")) {
            if (i == argc - 1) {
                printf("Error : Missing server name after -n argument\n");
                exit(-6);
            }
            if (!strcmp(argv[i + 1], "-p") || !strcmp(argv[i + 1], "-n")) {
                printf("Error : Missing server name after -n argument\n");
                exit(-6);
            }
            strcpy(serverName, argv[i + 1]);
            if (i == argc - 1 && PORT == -1) {
                printf("Error : argument -p is missing\n");
                exit(-6);
            }
        } else if (argc == 1) {
            printf("Error : -p argument missing\n");
            exit(-6);
        }
    }
    int socketEcoute = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;
    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;
    struct pollfd *polls = malloc(sizeof(struct pollfd));
    User *clients = NULL;
    char messageEnvoi[LG_MESSAGE];
    char messageRecu[LG_MESSAGE];
    int lus;
    int nbrePolls = 1;

    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }

    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    polls[0].fd = socketEcoute;
    polls[0].events = POLLIN;

    longueurAdresse = sizeof(struct sockaddr_in);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    if ((bind(socketEcoute, (struct sockaddr *) &pointDeRencontreLocal, longueurAdresse)) < 0) {
        perror("bind");
        exit(-2);
    }

    printf("Socket attachée avec succès !\n");

    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        exit(-3);
    }

    printf("Socket placée en écoute passive ...\n\n");

    while (1) {
        memset(messageEnvoi, 0x00, LG_MESSAGE * sizeof(char));
        memset(messageRecu, 0x00, LG_MESSAGE * sizeof(char));
        User *temp = clients;
        char getCmd[LG_MESSAGE * sizeof(char)] = {"\0"};
        char getArgs[2][LG_MESSAGE * sizeof(char)] = {"\0", {"\0"}};
        int return_value = poll(polls,nbrePolls,3000);

        if (return_value) {
            if (polls[0].revents && POLLIN) {
                socketDialogue = accept(socketEcoute, (struct sockaddr *) &pointDeRencontreDistant, &longueurAdresse);
                if (clients == NULL) {
                    clients = malloc(sizeof(User));
                    clients->socketClient = socketDialogue;
                    strcpy(clients->login, "");
                    clients->suiv = NULL;
                    temp = clients;
                } else {
                    while (temp->suiv != NULL) {
                        temp = temp->suiv;
                    }

                    temp->suiv = malloc(sizeof(User));
                    temp->suiv->socketClient = socketDialogue;
                    strcpy(temp->suiv->login, "");
                    temp = temp->suiv;
                    temp->suiv = NULL;
                }

                polls = realloc(polls, 1 + nbrePolls * sizeof(struct pollfd));
                polls[nbrePolls].fd = temp->socketClient;
                polls[nbrePolls].events = POLLIN;
                nbrePolls++;
            }

            for (int i = 1 ; i < nbrePolls ; i++) {
                if (polls[i].revents && POLLIN) {
                    for (int i = 1; i < nbrePolls; i++) {
                        if (polls[i].revents) {
                            socketDialogue = polls[i].fd;
                            break;
                        }
                    }

                    lus = read(socketDialogue, messageRecu, LG_MESSAGE * sizeof(char));
                    while (temp != NULL && temp->socketClient != socketDialogue) {
                        temp = temp->suiv;
                    }
                    switch (lus) {
                        case -1 :
                            perror("read");
                            close(socketDialogue);
                            exit(-5);
                        case 0 :
                            strcpy(messageEnvoi, "/mg serveur Le client ");
                            strcat(messageEnvoi, temp->login);
                            strcat(messageEnvoi, " s'est déconnecté.\n");
                            mg(clients, temp->login, messageEnvoi, polls, &nbrePolls);
                            disconnect(&clients, temp->login, polls, &nbrePolls);
                            break;
                        default:
                            if (messageRecu[0] != 47) {
                                break;
                            }

                            clients = cmdHandler(clients, temp, messageRecu, polls, &nbrePolls, greeting);
                            break;
                    }
                }
            }
        }
    }

    close(socketEcoute);

    return 0;
}