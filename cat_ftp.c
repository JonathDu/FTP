#include "csapp.h"
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>


void cat_ftp(int connfd)
{
    size_t n, m, nread;
    char bufFileName[MAXLINE];
    char bufIndexDebutLecture[MAXLINE];
    char bufFileContent[MAXLINE];
    rio_t rio;
    int indiceDebutLecture;
    int totalBytesTransfert = 0;

    Rio_readinitb(&rio, connfd);
    printf("******* Lecture nom fichier *******\n");
    n = Rio_readlineb(&rio, bufFileName, MAXLINE); //lecture du nom du fichier envoyé par le client
    printf("Nom du fichier : %s\n", bufFileName);

    printf("******* Lecture indice debut lecture *******\n");
    m = Rio_readlineb(&rio, bufIndexDebutLecture, MAXLINE); //lecture de l'indice de début de lecture du fichier envoyé par le client
    indiceDebutLecture = atoi(bufIndexDebutLecture);
    printf("Indice début lecture : %i\n", indiceDebutLecture);

    if (n != 0 && m != 0) //si bien recu
    {
        printf("Server received %u bytes\n", (unsigned int)(n + m));

        bufFileName[n - 1] = '\0'; //suppression du char de fin de ligne (necessaire pour le fopen)

        FILE *fileRead = fopen(bufFileName, "r"); //ouverture du fichier en lecture
        fseek(fileRead, indiceDebutLecture, SEEK_SET); //on positionne le curseur de lecture du fichier au bon endroit (a l'indice indiceDebutLecture, en partant du début du fichier)
        while ((nread = fread(bufFileContent, 1, sizeof(bufFileContent), fileRead)) > 0) //tant que l'on peut lire dans le fichier, on met la lecture dans bufFileContent
        {
            if (rio_writen(connfd, bufFileContent, nread) == -1) //on ecrit dans la socket et si il y a une erreur (coté client) => on arrete le transfert
            {
                printf("Erreur de communication avec le client\n");
                break;
            }
            totalBytesTransfert += nread; //useless ????
            sleep(3);
        }
    }
}
