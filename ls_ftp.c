#include "csapp.h"
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void ls_ftp(int clientFd, char *bufCmdParamClient)
{
    DIR *mydir;
    struct dirent *myfile;
    char bufFileName[MAXLINE];
    mydir = opendir(".");
    while ((myfile = readdir(mydir)) != NULL)
    {
        strcpy(bufFileName, myfile->d_name);
        strcat(bufFileName, "\t");
        rio_writen(clientFd, bufFileName, strlen(bufFileName));
    }

}