#include "csapp.h"
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>

void pwd_ftp(int clientFd)
{
    char cwd[MAXLINE];
    getcwd(cwd, sizeof(cwd));
    rio_writen(clientFd, cwd, strlen(cwd));
}
