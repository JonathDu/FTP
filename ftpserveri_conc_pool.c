#include "csapp.h"

#define MAX_NAME_LEN 256
#define NPROC 4

void cat_ftp(int connfd);

// METHODE CHOISIT : LE ACCEPT() EST DANS LE FILS => PLUS SIMPLE

int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    clientlen = (socklen_t)sizeof(clientaddr);
    listenfd = Open_listenfd(port);

    int i, pid;

    for (i = 0; i < NPROC; i++)
    {
        if ((pid = Fork()) == 0)
        {
            //Fils
            while (1)
            {
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //attend la connexion d'un client
                /* determine the name of the client */
                Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

                printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

                cat_ftp(connfd);

                Close(connfd);
            }
        }
    }

    for (i = 0; i < NPROC; i++)
        Wait(NULL);

    exit(0);
}
