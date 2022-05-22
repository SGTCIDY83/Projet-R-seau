#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "./header.h"
#include "./implement.c"

int main(int argc, char **argv) {
    int PORT = -1;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-p")) {
            if (i == argc - 1) {
                printf("Error : Missing port number after -p argument\n");
                exit(-6);
            } else if (!(PORT = atoi(argv[i + 1]))) {
                printf("Error : argument -p must be followed by an integer type of argument\n");
                exit(-6);
            }
        } else if (argc == 1) {
            printf("Error : -p argument missing\n");
            exit(-6);
        }
    }

    int descripteurSocket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in pointDeRencontreDistant;
    socklen_t longueurAdresse;
    char messageEnvoi[LG_MESSAGE];
    char messageRecu[LG_MESSAGE];
    char version[14] = "/version 0.1c\n";
    int ecrits, lus;
    int inbuf = 0; // combien d'octets sont dans le buffer
    int espace = 0; // combien d'espace il reste dans le buffer
    char *after;
    char *prevCmd = malloc(LG_MESSAGE * sizeof(char));
    char **prevArgs = malloc(2 * sizeof(char *));
    for (int i = 0; i < 2; i++) {
        prevArgs[i] = malloc(LG_MESSAGE * sizeof(char));
    }

    if (descripteurSocket < 0) {
        perror("socket");
        exit(-1);
    }

    printf("Socket créée avec succès ! (%d)\n", descripteurSocket);

    longueurAdresse = sizeof(pointDeRencontreDistant);
    memset(&pointDeRencontreDistant, 0x00, longueurAdresse);
    pointDeRencontreDistant.sin_family = PF_INET;
    pointDeRencontreDistant.sin_port = htons(PORT);
    inet_aton("127.0.0.1", &pointDeRencontreDistant.sin_addr);

    if ((connect(descripteurSocket, (struct sockaddr *) &pointDeRencontreDistant, longueurAdresse)) == -1) {
        perror("connect");
        close(descripteurSocket);
        exit(-2);
    }

    printf("Connexion au serveur réussie avec succès !\n");

    ecrits = write(descripteurSocket, version, strlen(version));
    switch (ecrits) {
        case -1 :
            perror("write");
            close(descripteurSocket);
            exit(-3);
        case 0 :
            fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
            close(descripteurSocket);
            return 0;
    }

    lus = read(descripteurSocket, messageRecu, LG_MESSAGE * sizeof(char));
    switch (lus) {
        case -1 :
            perror("read");
            close(descripteurSocket);
            exit(-3);
        case 0 :
            fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
            close(descripteurSocket);
            return 0;
        default:
            cmdHandler(messageRecu, prevCmd, prevArgs);
    }

    //set up variables for select()
    fd_set all_set, r_set;
    int maxfd = descripteurSocket + 1;
    FD_ZERO(&all_set);
    FD_SET(STDIN_FILENO, &all_set);
    FD_SET(descripteurSocket, &all_set);
    r_set = all_set;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    //on initialise after
    after = messageEnvoi;

    while (1) {
        memset(messageEnvoi, 0x00, LG_MESSAGE * sizeof(char));
        memset(messageRecu, 0x00, LG_MESSAGE * sizeof(char));
        r_set = all_set;

        //vérifier si on peut lire depuis STDIN ou la socket
        select(maxfd, &r_set, NULL, NULL, &tv);

        if (FD_ISSET(STDIN_FILENO, &r_set)) {
            if (buffer_message(messageEnvoi, &inbuf, &espace, after)) {
                //On envoie le message après lui avoir alloué l'espace
                ecrits = write(descripteurSocket, messageEnvoi, strlen(messageEnvoi));
                switch (ecrits) {
                    case -1 :
                        perror("write");
                        close(descripteurSocket);
                        exit(-3);
                    case 0 :
                        fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                        close(descripteurSocket);
                        return 0;
                }

                prevCmd = getCmd(messageEnvoi); //On récupère la commande
                prevArgs = getArgs(messageEnvoi, prevCmd); //On récupère le/les argument(s)
                if(!strcmp(prevCmd, "/mg")) {
                    printf("\x1b[1F"); // On va a la ligne précédente du terminal
                    printf("\x1b[2K"); // On efface la ligne
                    printf("\033[94mGLOBAL\033[32m You\033[0m>\033[93m %s\033[0m\n", prevArgs[0]);
                }
            }
        }

        if (FD_ISSET(descripteurSocket, &r_set)) {
            //On reçoit une réponse du serveur
            lus = read(descripteurSocket, messageRecu, LG_MESSAGE * sizeof(char));
            switch (lus) {
                case -1 :
                    perror("read");
                    close(descripteurSocket);
                    exit(-3);
                case 0 :
                    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                    close(descripteurSocket);
                    return 0;
                default:
                    cmdHandler(messageRecu, prevCmd, prevArgs); //On traite la réponse du serveur
                    break;
            }
        }
    }

    close(descripteurSocket);

    return 0;
}