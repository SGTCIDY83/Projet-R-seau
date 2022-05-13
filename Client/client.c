#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LG_MESSAGE 256

int main(){
   int descripteurSocket = socket(PF_INET, SOCK_STREAM, 0);
   struct sockaddr_in pointDeRencontreDistant;
   socklen_t longueurAdresse;
   char messageEnvoi[LG_MESSAGE];
   char messageRecu[LG_MESSAGE];
   int ecrits, lus;
   int retour;

   if(descripteurSocket < 0){
      perror("socket");
      exit(-1);
   } 

   printf("Socket créée avec succès ! (%d)\n", descripteurSocket);
   
   longueurAdresse = sizeof(pointDeRencontreDistant);
   memset(&pointDeRencontreDistant, 0x00, longueurAdresse);
   pointDeRencontreDistant.sin_family = PF_INET;
   pointDeRencontreDistant.sin_port = htons(IPPORT_USERRESERVED);
   inet_aton("127.0.0.1", &pointDeRencontreDistant.sin_addr);

   if((connect(descripteurSocket, (struct sockaddr *)&pointDeRencontreDistant, longueurAdresse)) == -1){
      perror("connect");
      close(descripteurSocket);
      exit(-2);
   }

   printf("Connexion au serveur réussie avec succès !\n");

   lus = read(descripteurSocket, messageRecu, LG_MESSAGE*sizeof(char));
   switch(lus){
      case -1 :
         perror("read");
         close(descripteurSocket);
         exit(-3);
      case 0 :
         fprintf(stderr, "La socket a été fermée par le serveur !\n\n"); 
         close(descripteurSocket);
         return 0;
      default:
         printf("%s", messageRecu);
         scanf("%s", messageEnvoi);
   }

   ecrits = write(descripteurSocket, messageEnvoi, strlen(messageEnvoi));
   switch(ecrits){
      case -1 :
         perror("write");
         close(descripteurSocket);
         exit(-3);
      case 0 :
         fprintf(stderr, "La socket a été fermée par le serveur !\n\n"); 
         close(descripteurSocket);
         return 0;
      default:
         printf("Login %s envoyé avec succès (%d octets)\n\n", messageEnvoi, ecrits);
   }

   while(1){
      char messageEnvoi[1024] = {"\0"};
      char messageRecu[1024] = {"\0"};
      
      printf("Envoyer : ");
      scanf("%s", messageEnvoi);
      strcat(messageEnvoi, "\n");

      ecrits = write(descripteurSocket, messageEnvoi, strlen(messageEnvoi));
      switch(ecrits){
         case -1 :
            perror("write");
            close(descripteurSocket);
            exit(-3);
         case 0 :
            fprintf(stderr, "La socket a été fermée par le serveur !\n\n"); 
            close(descripteurSocket);
            return 0;
         default:
            printf("Message %s envoyé avec succès (%d octets)\n\n", messageEnvoi, ecrits);
      }
   }
   
   close(descripteurSocket);

   return 0;
}