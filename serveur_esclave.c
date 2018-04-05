#include "csapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_NAME_LEN 256
#define NB_PROC 4

void get_ftp(int clientFd, char *bufCmdParamClient);
void ls_ftp(int clientFd);

int main(int argc, char **argv)
{
    int listenfd, serveurMaitreFd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    port = 2121;

    clientlen = (socklen_t)sizeof(clientaddr);
    listenfd = Open_listenfd(port);
    int i, pid;

    for (i = 0; i < NB_PROC; i++)
    {
        if ((pid = Fork()) == 0)
        {

            while (1)
            {
                char bufIPClient[MAXLINE];
                char bufCmdClient[MAXLINE];
                char *bufCmdNameClient = malloc(sizeof(char) * MAXLINE);
                char *bufCmdParamClient = malloc(sizeof(char) * MAXLINE);

                printf("***********************************************************************\n");
                printf("\tESCLAVE : \t Attente d'une connexion du serveur maitre ...\n");
                /////////////////////////////////////////////
                // CONNEXION AU SERVEUR MAITRE
                rio_t rio;

                serveurMaitreFd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //attend la connexion du serveur maitre
                Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

                Rio_readinitb(&rio, serveurMaitreFd);
                printf("\tESCLAVE : \t Serveur esclave connecté au serveur maitre %s (%s)\n", client_hostname, client_ip_string);
                //
                /////////////////////////////////////////////

                ////////////////////////////////////////////
                // RECEPTION DES DONNEES DU SERVEUR MAITRE
                printf("\tESCLAVE : \t ******* Lecture IP client ... *******\n");
                Rio_readlineb(&rio, bufIPClient, MAXLINE); //lecture de la commande envoyée par le client (ex : get fichier1.txt)
                printf("\tESCLAVE : \t Adresse IP : %s\n", bufIPClient);
                printf("\tESCLAVE : \t ******* Lecture cmd client ... *******\n");
                Rio_readlineb(&rio, bufCmdClient, MAXLINE);   //lecture de la commande envoyée par le client (ex : get fichier1.txt)
                bufCmdNameClient = strtok(bufCmdClient, " "); //on recupere le nom de la commande (ex : get, ls, cd)
                bufCmdParamClient = strtok(NULL, " ");        //on recupere le parametre de la commande (ex : fichier1.txt)
                printf("\tESCLAVE : \t Nom commande client : %s\n", bufCmdNameClient);
                printf("\tESCLAVE : \t Param commande client : %s\n", bufCmdParamClient);
                //
                ////////////////////////////////////////////

                ////////////////////////////////////////////
                // CONNEXION AVEC LE CLIENT
                int clientFd = Open_clientfd(bufIPClient, 2126); //ouverture de la connexion avec le client
                //
                ////////////////////////////////////////////

                if (strcmp(bufCmdNameClient, "get") == 0)
                {
                    get_ftp(clientFd, bufCmdParamClient);
                }
                else if (strcmp(bufCmdNameClient, "ls:0\n") == 0)
                {
                    ls_ftp(clientFd);
                }
                else if (strcmp(bufCmdNameClient, "pwd") == 0)
                {
                    printf("TODO : pwd\n");
                }
                else if (strcmp(bufCmdNameClient, "cd") == 0)
                {
                    printf("TODO : cd\n");
                }

                Close(serveurMaitreFd); //fermeture de connexion avec le serveur maitre
                Close(clientFd);        //fermeture de connexion avec le client

                printf("***********************************************************************\n");
            }
        }
    }

    for (i = 0; i < NB_PROC; i++)
        Wait(NULL);

    exit(0);
}
