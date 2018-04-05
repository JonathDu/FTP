#include "csapp.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define TRANSFER_SIZE 16

void my_get_ftp(char bufCommande[], char indiceFile[], int listenfd);

size_t totalBytesTransfert = 0;
char *host, bufCommande[MAXLINE], *bufFileName, *bufCmdName;
int port;
int serveurMaitreFd, serveurEsclaveFd;

char *substring(char *string, int position, int length)
{
    char *pointer;
    int c;

    pointer = malloc(length + 1);

    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }

    for (c = 0; c < length; c++)
    {
        *(pointer + c) = *(string + position - 1);
        string++;
    }

    *(pointer + c) = '\0';

    return pointer;
}

int file_exist(const char *filename)
{
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
    close(serveurEsclaveFd);
    close(serveurMaitreFd);
}

int main(int argc, char **argv)
{

    bufFileName = malloc(sizeof(char) * MAXLINE);
    bufCmdName = malloc(sizeof(char) * MAXLINE);
    char indiceFile[MAXLINE]; //indice de debut de lecture du fichier

    Signal(SIGINT, handler); //si il y a un SIGINT -> execution de la fonction handler() avant de quitter

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    int listenfd = Open_listenfd(2123);          //ouverture du socket de reception de données (avec serveur esclave, sur le port 2122)

    printf("Client connected to server OS\n");

    DIR *mydir;
    struct dirent *myfile;
    mydir = opendir(".");

    ////////////////////////////////////////////////////
    //// MODE "FIN TRANSFERT"///////////////////////////
    //
    while ((myfile = readdir(mydir)) != NULL) //parcour de tous les fichiers
    {
        char *bufFileType = strrchr(myfile->d_name, '.');             //recupere l'extension du fichier
        if (bufFileType != NULL && strcmp(bufFileType, ".part") == 0) //si le fichier est de type .part
        {
            char *bufRealFileName = substring(myfile->d_name, 1, strlen(myfile->d_name) - 5); //suppression du ".part"
            FILE *frecup = Fopen(myfile->d_name, "r");                                        //ouverture du fichier
            fseek(frecup, 0L, SEEK_END);                                                      //position le curseur d'ecriture a la fin
            sprintf(indiceFile, "%li\n", ftell(frecup));
            fclose(frecup);
            char bufCmd[MAXLINE];
            sprintf(bufCmd, "%s %s\n", "get", bufRealFileName);
            printf("Fin de transfert de %s... \n", bufRealFileName);
            my_get_ftp(bufCmd, indiceFile, listenfd);
        }
    }
    //
    ////////////////////////////////////////////////////

    while (1)
    {
        /////////////////////////////////////////////
        //// MODE "NORMAL"///////////////////////////
        //
        totalBytesTransfert = 0;
        printf("ftp>");
        if (Fgets(bufCommande, MAXLINE, stdin) != NULL) //saisie de l'utilisateur (resultat dans bufCommande)
        {
            if (strcmp(bufCommande, "bye\n") == 0)
            {
                Close(listenfd);
                exit(0);
            }
            indiceFile[0] = '0';
            indiceFile[1] = '\n';

            my_get_ftp(bufCommande, indiceFile, listenfd);
        }
        //
        /////////////////////////////////////////////
        /////////////////////////////////////////////
    }

    exit(0);
}

void my_get_ftp(char bufCommande[], char indiceFile[], int listenfd)
{
    serveurMaitreFd = Open_clientfd(host, port); //ouverture de la connexion

    clock_t t1, t2;
    float temps;
    char bufResContent[MAXLINE];
    int size;

    //////////////////////////////
    // ENVOIS AU SERVEUR /////////
    bufCommande[strlen(bufCommande) - 1] = '\0';                   //suppression du char de fin de ligne
    strcat(bufCommande, ":");                                      //creation de "fileName:"
    strcat(bufCommande, indiceFile);                               //creation de "fileName:index"
    Rio_writen(serveurMaitreFd, bufCommande, strlen(bufCommande)); //envoi "fileName:index" au serveur
    //
    //////////////////////////////

    //////////////////////////////////////////////////////////
    // RECEPTION DES DONNEES ENVOYEES PAR LE SERVEUR /////////
    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);

    serveurEsclaveFd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //attend la connexion d'un serveur esclave
    t1 = clock();                                //enregistre l'heure de début du transfert
    char *temp = malloc(sizeof(char) * MAXLINE); //recupération du nom du fichier
    bufCmdName = strtok(bufCommande, " ");       //recupération du nom de la commande
    if (strcmp(bufCmdName, "get") == 0)
    {
        temp = strtok(NULL, " ");        //recupération du nom du fichier
        bufFileName = strtok(temp, ":"); //recupération du nom du fichier
        char *bufFileNamePart = malloc(sizeof(char) * MAXLINE);
        strcpy(bufFileNamePart, bufFileName);
        strcat(bufFileNamePart, ".part");
        int f = open(bufFileNamePart, O_RDWR | O_CREAT | O_APPEND, 0666);              //ouvre le fichier nommée bufFileName (créé le fichier avec les droits -rw-r--r-- si il n'existe pas)
        while ((size = Rio_readn(serveurEsclaveFd, bufResContent, TRANSFER_SIZE)) > 0) //tant qu'il y a des données serveur a lire, on lit et on met les TRANSFER_SIZE bytes dans bufFileContent
        {
            totalBytesTransfert = totalBytesTransfert + strlen(bufResContent); //calcul du nombre de bytes transférés
            Write(f, bufResContent, size);                                     //on ecrit le bloc de bytes dans le fichier precedement ouvert
        }
        Close(f); //fermeture du fichier
        rename(bufFileNamePart, bufFileName);
    }
    else
    {
        while ((size = Rio_readn(serveurEsclaveFd, bufResContent, TRANSFER_SIZE)) > 0) //tant qu'il y a des données serveur a lire, on lit et on met les TRANSFER_SIZE bytes dans bufFileContent
        {
            totalBytesTransfert = totalBytesTransfert + strlen(bufResContent); //calcul du nombre de bytes transférés
            printf("%s", bufResContent);                                       //on ecrit le bloc de bytes dans la sortie std
        }
        printf("\n");
    }
    t2 = clock();                              //enregistre l'heure de fin de transfert
    temps = (float)(t2 - t1) / CLOCKS_PER_SEC; //calcul du temps de transfert
    printf("Transfer successfully complete.\n");
    printf("%li bytes received in %f seconds (%f Kbytes/s).\n", totalBytesTransfert, temps, ((float)(totalBytesTransfert / 1000) / temps));
    Close(serveurEsclaveFd);
    Close(serveurMaitreFd);
    //
    //////////////////////////////////////////////////////////
}
