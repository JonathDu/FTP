#include "csapp.h"
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>

#define TRANSFER_SIZE 32

void get_ftp(int clientFd, char *bufCmdParamClient)
{
    char *bufFileName = malloc(sizeof(char) * MAXLINE);
    int indiceDebutLecture;
    size_t nread;

    printf("\tESCLAVE -GET- : \t ******* Lecture indice debut lecture *******\n");
    bufFileName = strtok(bufCmdParamClient, ":"); //on recupere le nom du fichier
    indiceDebutLecture = atoi(strtok(NULL, ":")); //on recupere l'indexe de debut de lecture du fichier
    printf("\tESCLAVE -GET- : \t Nom du fichier : %s\n", bufFileName);
    printf("\tESCLAVE -GET- : \t Indice début lecture : %i\n", indiceDebutLecture);

    FILE *fileRead = Fopen(bufFileName, "r");      //ouverture du fichier en lecture
    fseek(fileRead, indiceDebutLecture, SEEK_SET); //on positionne le curseur de lecture du fichier au bon endroit (a l'indice indiceDebutLecture, en partant du début du fichier)

    char bufFileContent[TRANSFER_SIZE];

    while ((nread = fread(bufFileContent, 1, TRANSFER_SIZE, fileRead)) > 0) //tant que l'on peut lire dans le fichier, on met la lecture dans bufFileContent
    {
        if (rio_writen(clientFd, bufFileContent, nread) == -1) //on ecrit dans la socket et si il y a une erreur (coté client) => on arrete le transfert
        {
            printf("Erreur de communication avec le client\n");
            break;
        }
    }
}