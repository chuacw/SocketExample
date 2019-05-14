/*
Example developed and tested on Ubuntu Linux 18.04.2.

This example shows how to trigger select to return immediately by
sennding an OOB message to the receiving party.

Works on Linux and Android
The equivalent API code also works on Windows except that Windows Firewall is
triggered.

Enjoy!

chuacw, 
14 May 2019,
Singapore.

*/
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server, listenaddr;
    bzero((char *)&server, sizeof(server));

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(0);

    int rc = bind(serverSocket, (struct sockaddr *)&server, sizeof(server));
    if (rc >= 0)
    {
        printf("Server socket bound.\n");
    }
    else
    {
        printf("Failed to bind server socket.\n");
    }

    rc = listen(serverSocket, 1);
    if (rc >= 0)
    {
        printf("Server socket listening.\n");
    }
    else
    {
        printf("Failed to listen on server socket.\n");
    }

    // socklen_t len = sizeof(listenaddr);
    // rc = getsockname(serverSocket, (struct sockaddr *)&listenaddr, &len);
    socklen_t len = sizeof(listenaddr);
    rc = getsockname(serverSocket, (struct sockaddr *)&listenaddr, &len);
    int myPort = ntohs(listenaddr.sin_port);
    if (rc >= 0)
    {
        printf("Got socket connection address.\n");
    }

    if (listenaddr.sin_addr.s_addr == INADDR_ANY)
    {
        listenaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    rc = connect(clientSocket, (struct sockaddr *)&listenaddr, len);
    if (rc >= 0)
    {
        printf("Socket connected.\n");
    }

    int accepted = accept(serverSocket, (struct sockaddr *)&listenaddr, &len);
    if (accepted > 0)
    {
        printf("Accepted connection!\n");
    }

    char *oob_data = (char *)"chuacw";

    // Without this, the call to select will timeout.
    rc = send(clientSocket, oob_data, 6, MSG_OOB);

    fd_set master_fds, except_fds;
    FD_ZERO(&master_fds);
    FD_SET(accepted, &master_fds);

    memcpy(&except_fds, &master_fds, sizeof(master_fds));

    printf("Entering select...\n");

    int max_fd = accepted;
    timeval timeout;
    timeout.tv_sec = 10 * 60; // 10 minutes, but this will return almost immediately.
    timeout.tv_usec = 0;
    rc = select(max_fd + 1, NULL, NULL, &except_fds, &timeout);

    switch (rc)
    {
    case -1:
        printf("Error occurred during select.\n");
        break;
    case 0:
        printf("Timeout occurred during select.\n");
        break;
    default:
        printf("Conditions triggered during select.\n");
    }

    if (accepted > 0)
        close(accepted);
    if (clientSocket > 0)
        close(clientSocket);
    if (serverSocket > 0)
        close(serverSocket);

    return 0;
}