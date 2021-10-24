#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <pthread.h>   // for threading, link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "html_parser.h"
#include <sys/syscall.h>
#include <errno.h>


#define BUFFER_SIZE 65535


void *tcp_handler(void *sock_desc, void *arg)
{
    int request;
    char client_reply[BUFFER_SIZE], *request_lines[3];
    char *size;

    // Get the socket descriptor.
    int sock = *((int *)sock_desc);

    // Get the request.
    request = recv(sock, client_reply, BUFFER_SIZE, 0);

    if (request < 0) // Receive failed.
    {
        printf("Recv failed\n");
    }
    if (request == 0) // receive socket closed. Client disconnected upexpectedly.
    {
        printf("Client disconnected upexpectedly.\n");
    }
    else // Message received.
    {

        int size = GET_SIZE(client_reply);
        printf("%s", client_reply);

        if(size < 1)
        {
            char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> URL must be a number </body></html>";
            printf("%s\n", message);
            write(sock, message, strlen(message));
        }
        if (!GET_CONTROL(client_reply) && !POST_CONTROL(client_reply) && !PUT_CONTROL(client_reply))
        {
            char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>You need to use GET request </body></html>";
            printf("%s\n", message);
            write(sock, message, strlen(message));
        }
        if (PUT_CONTROL(client_reply) > 0)
        {
            char *message = "HTTP/1.0 501 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> Not implemented </body></html>";
            printf("%s\n", message);
            write(sock, message, strlen(message));
        }
        if (POST_CONTROL(client_reply) > 0)
        {
            char *message = "HTTP/1.0 501 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> Not implemented </body></html>";
            printf("%s\n", message);
            write(sock, message, strlen(message));
        }
        if (size < 100 || 20000 < size)
        {
            char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>You need to request file sized 100 between 20000.</body></html>";
            printf("%s\n", message);
            write(sock, message, strlen(message));
        }
        else
        {
            
            /* constructing the response message */ 
            char *mhead = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><head><title> I am ";
            char *mhead_cont = "bytes long </title></head><body>";
            char number[10];
            sprintf(number, "%d", size);
            char *mend = "</body></html>";
            char *temp = (char *)malloc((size + strlen(mhead) + strlen(mend) + strlen(mhead_cont)) * sizeof(char));
            strcat(temp, mhead);
            strcat(temp, number);
            strcat(temp,mhead_cont);

            printf("%s\n", temp);
            for (int i = 0; i < size; i++)
            {
                temp = str_concatenator(temp, 'a', 2);
            }
            strcat(temp, mend);
            write(sock, temp, strlen(temp));
        }
    }
    free(sock_desc);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = -1;
    return 0;
}

int main(int argc, char *argv[])
{


    /* variables for socket */

    int sock_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;

    /* id for thread */

    pthread_t tid;

    /*  usage control */

    if (argc != 2)
    {
        printf("Correct usage is <http_multithread_server> <PORT_NO> \n");
        return 1;
    }

     /* assinging the argument */

    int PORT_NO = atoi(argv[1]);

    /* creating socket */

    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }

    /* server is socket struct, filling its attiributes */

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);

    /* bind socket with its id */

    if (bind(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Binding failed\n");
        return 1;
    }

    /* listens requests */

    listen(sock_desc, 20);

    printf("Waiting for incoming connections...");

    /* each requests new thread creating in the while loop */

    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(sock_desc, (struct sockaddr *)&client, (socklen_t *)&c))) // Accept the connection.
    {

        printf("Connection accepted \n");
        new_sock = malloc(1);
        *new_sock = new_socket;

        /* thread create with argument of socket */

        if (pthread_create(&tid, NULL, (void *)tcp_handler, new_sock))
        {
            printf("Thread couldn't created \n");
        }

        /* no thread will finish code before job is done */
        pthread_join(tid, NULL);
    }

    return 0;
}