/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <time.h>

#define TRANSFER_SIZE 16
char *LOG_FILE_NAME = "logErreurTransfert";

void my_get_ftp(int clientfd, char bufFileName[], char indiceFile[]);


size_t totalBytesTransfert = 0;
char *host, bufFileName[MAXLINE];

int file_exist(const char *filename)
{
    /* try to open file to read */
    FILE *file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void handler(int sig)
{
    FILE *f = Fopen(LOG_FILE_NAME, "w");
    fprintf(f, "%s\n%s\n%li\n", host, bufFileName, totalBytesTransfert);
    Fclose(f);
    exit(0);
}

int main(int argc, char **argv)
{
    int clientfd, port;
    rio_t rio;

    char indiceFile[MAXLINE]; //indice de debut de lecture du fichier

    Signal(SIGINT, handler); //si il y a un SIGINT -> execution de la fonction handler() avant de quitter

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    clientfd = Open_clientfd(host, port); //ouverture de la connexion

    printf("Client connected to server OS\n");

    /////////////////////////////////////////////
    //// FIN TRANSFERT FICHIER APRES ERREUR//////
    //
    if (file_exist(LOG_FILE_NAME)) //si un fichier log existe => on doit finir le transfert avant toute autre action
    {
        char bufLogHostName[MAXLINE];
        FILE *flog = fopen(LOG_FILE_NAME, "r"); //ouverture du fichier de log
        fgets(bufLogHostName, MAXLINE, flog);   //recupere le host dans le fichier
        if (strcmp(bufLogHostName, host))       //si ce fichier contient le meme host que celui passé en param au programme
        {
            printf("Un fichier n'a pas finit d'être envoyer.\n");
            printf("Suite de l'envoi ...\n");

            fgets(bufFileName, MAXLINE, flog);           //recupere, dans le fichier log, le nom du fichier a finir de transferer
            fgets(indiceFile, sizeof(indiceFile), flog); //recupere, dans le fichier log, le nombre de bits qu'on avait deja lu avant le bug

            my_get_ftp(clientfd, bufFileName, indiceFile);

            remove(LOG_FILE_NAME);
        }
    }
    //
    /////////////////////////////////////////////
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    //// MODE "NORMAL"///////////////////////////
    //
    printf("ftp>");
    Rio_readinitb(&rio, clientfd); //Associate a descriptor with a read buffer (in struct rio) and reset buffer

    //TODO : analyse du texte saisi par l'utilisateur (get ...)
    if (Fgets(bufFileName, MAXLINE, stdin) != NULL) //saisie de l'utilisateur (resultat dans bufFileName)
    {
        indiceFile[0] = '0';
        indiceFile[1] = '\n';

        my_get_ftp(clientfd, bufFileName, indiceFile);
    }
    Close(clientfd);
    //
    /////////////////////////////////////////////
    /////////////////////////////////////////////
    exit(0);
}

void my_get_ftp(int clientfd, char bufFileName[], char indiceFile[])
{
    clock_t t1, t2;
    float temps;
    char bufFileContent[MAXLINE];

    //////////////////////////////
    // ENVOIS AU SERVEUR /////////
    Rio_writen(clientfd, bufFileName, strlen(bufFileName)); //envoi le nom du fichier au serveur
    Rio_writen(clientfd, indiceFile, strlen(indiceFile));   //envoi l'indice ou la lecture du fichier doit commencer
    //
    //////////////////////////////

    //////////////////////////////////////////////////////////
    // RECEPTION DES DONNEES ENVOYEES PAR LE SERVEUR /////////
    t1 = clock();                                                 //enregistre l'heure de début du transfert
    bufFileName[strlen(bufFileName) - 1] = '\0';                  //suppression du char de fin de ligne (necessaire pour le open)
    int f = open(bufFileName, O_RDWR | O_CREAT | O_APPEND, 0666); //ouvre le fichier nommée bufFileName (créé le fichier avec les droits -rw-r--r-- si il n'existe pas)
    int size;
    while ((size = Rio_readn(clientfd, bufFileContent, TRANSFER_SIZE)) > 0) //tant qu'il y a des données serveur a lire, on lit et on met les TRANSFER_SIZE bytes dans bufFileContent
    {
        totalBytesTransfert = totalBytesTransfert + strlen(bufFileContent); //calcul du nombre de bytes transférés
        Write(f, bufFileContent, size);                                     //on ecrit le bloc de bytes dans le fichier precedement ouvert
    }
    Close(f);                                  //fermeture du fichier
    t2 = clock();                              //enregistre l'heure de fin de transfert
    temps = (float)(t2 - t1) / CLOCKS_PER_SEC; //calcul du temps de transfert
    printf("Transfer successfully complete.\n");
    printf("%li bytes received in %f seconds (%f Kbytes/s).\n", totalBytesTransfert, temps, ((float)(totalBytesTransfert / 1000) / temps));
    //
    //////////////////////////////////////////////////////////
}
