#include "csapp.h"
#include<pthread.h>

#define MAX_NAME_LEN 256
#define NB_SERVEUR_ESCLAVE 2
#define NB_PROC 4

int getSizeToRead(int totalRead, int fileSize);

int main(int argc, char **argv)
{
    int listenfd, ClientFd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    rio_t rio;
    char bufCmdClient[MAXLINE];
    char bufIPClient[MAXLINE];

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <ip_serv_esclave_1> <ip_serv_esclave_2>\n", argv[0]);
        exit(0);
    }
    port = 2121;

    clientlen = (socklen_t)sizeof(clientaddr);
    listenfd = Open_listenfd(port);

    ////////////////////////////////////////
    //MISE EN PLACE DES SERVEURS ESCLAVES
    int serveurEsclaveId = 0;
    char* listIPEsclaves[NB_SERVEUR_ESCLAVE];// = { argv[1], argv[2] };
    listIPEsclaves[0] = argv[1];
    listIPEsclaves[1] = argv[2];
    //
    ////////////////////////////////////////
    int i, pid;

    for (i = 0; i < NB_PROC; i++)
    {
        if ((pid = Fork()) == 0)
        {
            while (1)
            {
                printf("***********************************************************************\n");
                printf("\tMAITRE : \t Attente d'une connexion d'un client ...\n");

                ClientFd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //attend la connexion d'un client
                Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

                printf("\tMAITRE : \t Serveur Maitre connecté au client %s (%s)\n", client_hostname, client_ip_string);

                ////////////////////////////////////
                // RECEPTION DES COMMANDES CLIENTS
                Rio_readinitb(&rio, ClientFd);
                Rio_readlineb(&rio, bufCmdClient, MAXLINE); //lecture de la commande envoyée par le client (ex : get fichier1.txt)
                printf("\tMAITRE : \t Commande du client : %s\n", bufCmdClient);
                //
                ////////////////////////////////////

                ////////////////////////////////////////////
                //SELECTION DU SERVEUR ESCLAVE
                serveurEsclaveId ++;
                if(serveurEsclaveId >= NB_SERVEUR_ESCLAVE){
                    serveurEsclaveId = serveurEsclaveId - NB_SERVEUR_ESCLAVE;
                }
                printf("%i\n", serveurEsclaveId);

                //(serveurEsclaveId + 1) % (NB_SERVEUR_ESCLAVE-1); //round robin
                int serveurEsclaveFd = Open_clientfd(listIPEsclaves[serveurEsclaveId], 2122);  //ouverture de la connexion avec un serveur esclave
                printf("\tMAITRE : \t Fd serveur esclave choisit : \n\t\t\t\t - FD : %i\n\t\t\t\t - Num list: %i\n", serveurEsclaveFd, serveurEsclaveId);
                //
                ////////////////////////////////////////////

                ////////////////////////////////////////////
                //ENVOI IP CLIENT A L'ESCLAVE
                printf("\tMAITRE : \t Envoi de l'IP au serveur esclave... \n");
                strcpy(bufIPClient, client_ip_string);
                bufIPClient[strlen(bufIPClient)] = '\n';
                Rio_writen(serveurEsclaveFd, bufIPClient, strlen(bufIPClient)); //envoi de la commande du client au serveur esclave
                printf("\tMAITRE : \t Envoi de l'IP au serveur esclave terminé \n");
                //
                ////////////////////////////////////////////

                ////////////////////////////////////////////
                //ENVOI COMMANDE AU SERVEUR ESCLAVE
                printf("\tMAITRE : \t Envoi de la commande au serveur esclave... \n");
                Rio_writen(serveurEsclaveFd, bufCmdClient, strlen(bufCmdClient)); //envoi de la commande du client au serveur esclave
                printf("\tMAITRE : \t Envoi de la commande au serveur esclave terminé \n");
                //
                ////////////////////////////////////////////
                Close(ClientFd);
            }
            printf("***********************************************************************\n");
        }
    }

    for (i = 0; i < NB_PROC; i++)
        Wait(NULL);

    exit(0);
}
