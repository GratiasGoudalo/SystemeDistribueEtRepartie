
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>



#define MY_ADDR "127.0.0.4"
#define MY_PORT 6004
#define LISTEN_BACKLOG 50

int main(int argc, char *argv[]){
   int sfd; // socket du serveur
   int cfd; // socket du client
   int number=0; //Number for initialisation of sharing memory 
   struct sockaddr_in my_addr; // socket addr du serveur 
   struct sockaddr_in peer_addr; // socket addr d'un client
   FILE* fichier = NULL; //Le nom du fichier à recevoir
   pid_t pid1, pid2; //Les pids pour les deux processus
   char *argp1[] = {"proc1", NULL, NULL};
   char *argp2[] = {"proc2", NULL, NULL};//Pour les chargumentsdes processus
   int status;
   
   socklen_t peer_addr_size; // la taille du sock
   pid_t child;

   char buffer[10]; // pour recevoir la salutation du client

   sfd = socket(AF_INET, SOCK_STREAM, 0); // création de la socket
   if (sfd < 0){ // Une erreur s'est produite la socket n'a pas pu être créer
      printf("Le SE n'a pas pu créer la socket %d\n", sfd);
      exit(-1);
   }
   
   //Sharing memory creation
   //Generate a key
    key_t key = ftok("memory",65);
    //Take identifier of memory
    int memoId = shmget(key,1024,0666|IPC_CREAT);
    //Attach the shared memory
    char *str = (char*) shmat(memoId,(void*)0,0);
    sprintf(str,"%d",number);

   /* La structure est remise à 0 car elle doit servir à stocker
    * l'identité du serveur*/
   memset(&my_addr, 0, sizeof(struct sockaddr_in));
   
    /* donne une identite a la socket. */
   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons (MY_PORT);
   inet_aton(MY_ADDR, (struct in_addr *)&my_addr.sin_addr.s_addr);
  
   /* on demande au SE de confirmer l'identité de la socket
    * Cela autorise le SE d'exploitation à forwarder les requête
    * Sur le port 56789 à ce processus */
   if (bind(sfd, (struct sockaddr *) &my_addr,
                   sizeof(struct sockaddr_in)) < 0){
       printf("bind error\n"); // l'association a echouée
       exit(-1);
  }

   /* on demande au SE de définir le nombre de clients 
    * que le serveur peut 
    * traiter en même temps dans sa file d'attente */
   if (listen(sfd, LISTEN_BACKLOG) < -1)
               perror("listen\n");

   // on se bloque en attendant les connexion des clients
   peer_addr_size = sizeof(struct sockaddr_in);
   while(1){
         cfd = accept(sfd, (struct sockaddr *) &peer_addr,
                   &peer_addr_size);  
        if (cfd < 0){
            perror("accept\n");
            exit(-1); // Une erreur s'est produite
        }
        /* Nous créons un fils pour gérer ce client */
        child = fork();
        if(child < 0){ // le fils n'a pas pu être créé
           perror("errreur de création du fils\n");
        }
        if(child==0){
             /* Nous sommes dans le fils nous attendons la requête du client */
             printf("indentité du client %d\n", peer_addr.sin_port);
             /*Lecture des donnees envoyées par le client*/
             while(read(cfd, buffer, 10))
                 printf("%s", buffer);
             printf("\n");
             pid1 = fork();
             if(pid1<0){
                printf("Error the first son haven't created\n");
            } else{
                if(pid1==0){
                    execv("./proc1",argp1);
                }
                else{
                    pid2 = fork();
                    if(pid2<0){
                        printf("Error the second son haven't created\n");
                    }
                    else{
                        if(pid2==0){
                            execv("./proc2",argp2);
                        }
                        else{
                            pid1 = wait(&status);
                            sleep(2);
                            pid2 = wait(&status);
                            printf("Execution of process finished.\n");
                            //detach from shared memory
                            shmdt(str);
                            // destroy the shared memory
                            shmctl(memoId,IPC_RMID,NULL);
                        }
                    }
                }
            }
                
                break;
           }

        else{
          /*Dans le père: le père attent un autre client*/
          close(cfd);
        }
  }
}
