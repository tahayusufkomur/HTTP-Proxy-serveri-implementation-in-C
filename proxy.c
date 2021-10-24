#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#define MAX_SIZE 65535
#include "html_parser.h"
struct cache_node
{
    char response_msg[MAX_SIZE];
    int full;
};

struct thread_arg
{

    struct cache_node *st;
    int client_fd;
    char ip[100];
    char port[100];
    char hostname[100];
};

void *proxy_handler(void *arg)
{

    char buffer[MAX_SIZE];
    struct thread_arg *info = (struct thread_arg *)arg;
    struct sockaddr_in server_sd;
    int server_fd = 0;
    int check_read = 0;
    char *URL;
    struct cache_node *st = info->st;

    /* server socket check */

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Server socket could not created. \n");
        return 0;
    }

    /* set server socket variables */

    memset(&server_sd, 0, sizeof(server_sd));
    server_sd.sin_family = AF_INET;
    server_sd.sin_port = htons(atoi(info->port));
    server_sd.sin_addr.s_addr = inet_addr(info->ip);

    /* connection check */

    if ((connect(server_fd, (struct sockaddr *)&server_sd, sizeof(server_sd))) < 0)
    {
        printf("server connection not established");
        return 0;
    }

    /* reading clients request */

    memset(&buffer, '\0', sizeof(buffer));
    check_read = read(info->client_fd, buffer, sizeof(buffer));

    /* parse URL */

    int size = GET_SIZE(buffer);
    char str[10000];
    sprintf(str, "%d", size);
    URL = (char *)malloc((strlen(info->hostname) + strlen(info->port) + strlen(str) + 2) * sizeof(char));
    strcat(URL, info->hostname);
    URL = str_concatenator(URL, ':', 2);
    strcat(URL, info->port);
    URL = str_concatenator(URL, '/', 2);
    strcat(URL, str);

    printf("REQUESTED URL is: %s\n\n", URL);

    /* cache check, is request in cache then send response from cache */

    if (st[size].full > 0)
    {
        write(info->client_fd, st[size].response_msg, sizeof(st[size].response_msg));
        printf("CACHED RESPONSE USED \n\n SERVER RESPONSE IS: %s \n\n", st[size].response_msg);
        return NULL;
    }

    /* URL length check */

    if (strlen(URL) > 9999)
    {
        char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> URL IS TOO LONG </body></html>";
        write(info->client_fd, message, sizeof(message));
        return NULL;
    }

    /* send clients request to the server*/

    write(server_fd, buffer, sizeof(buffer));

    /* receive servers response*/

    memset(&buffer, '\0', sizeof(buffer));
    check_read = read(server_fd, buffer, sizeof(buffer));

    /* server down check */

    if (check_read <= 0)
    {
        char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> Server is down. </body></html>";
        write(info->client_fd, message, sizeof(message));
        return NULL;
    }

    /* If request is not in the cache, then cache it and print the response */

    if (st[size].full < 0)
    {
        strcpy(st[size].response_msg, buffer);
        st[size].full = 1;
        printf("REQUEST & RESPONSE CACHED \n");
        printf("\nSERVER RESPONSE IS: \n\n %s", st[size].response_msg);
    }

    /* send response back to client */

    write(info->client_fd, buffer, sizeof(buffer));

    return 0;
}

/* this function gets hostname and convert it to an ip address */

int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        // get the host info
        printf("Couldn't make it");
        return -1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 1;
}

int main(int argc, char const *argv[])
{

    /* struct for cache */

    struct cache_node *st = malloc(20000 * sizeof(struct cache_node));
    for (int i = 0; i < 20000; i++)
    {
        st[i].full = -1;
    }

    /*  usage control */

    if (argc != 4)
    {
        printf("Correct usage is  <DOMAIN NAME> <proxy_server PORT> <PROXY PORT> \n");
        return -1;
    }

    /* id for thread */

    pthread_t tid;

    /* ip for ip address */

    char ip[100];

    /* assinging arguments */
    char hostname[100];
    char server_port[100];
    char proxy_port[100];

    strcpy(hostname, argv[1]);
    strcpy(server_port, argv[2]);
    strcpy(proxy_port, argv[3]);

    /* converting domainname to ip addres */

    if (hostname_to_ip(hostname, ip) == -1)
    {
        printf("Couldn't resolve domain name.");
        return -1;
    }

    printf("Domains ip adress: %s\n", ip);
    printf("Server port: %s\n", server_port);
    printf("Proxy port: %s\n", proxy_port);
    int proxy_fd = 0, client_fd = 0;

    /* struct for proxy_server */

    struct sockaddr_in proxy_sd, client;

    /* proxy socket create check */
    if ((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nFailed to create socket");
        return -1;
    }

    printf("Proxy socket created with id:  %d\n", proxy_fd);

    /* constructing the socket attiributes */

    memset(&proxy_sd, 0, sizeof(proxy_sd));

    proxy_sd.sin_family = AF_INET;
    proxy_sd.sin_port = htons(atoi(proxy_port));
    proxy_sd.sin_addr.s_addr = INADDR_ANY;

    if ((bind(proxy_fd, (struct sockaddr *)&proxy_sd, sizeof(proxy_sd))) < 0)
    {
        printf("Failed to bind a socket");
    }

    if ((listen(proxy_fd, SOMAXCONN)) < 0)
    {
        printf("Failed to listen");
    }
    int c = sizeof(struct sockaddr_in);

    printf("Waiting for incoming connections... \n");
    while (1)
    {
        client_fd = accept(proxy_fd, (struct sockaddr *)&client, (socklen_t *)&c);
        printf("Waiting for incoming connections... \n");
        if (client_fd > 0)
        {
            struct thread_arg *item = malloc(sizeof(struct thread_arg));
            item->client_fd = client_fd;
            item->st = st;
            strcpy(item->hostname, hostname);
            strcpy(item->ip, ip);
            strcpy(item->port, server_port);
            pthread_create(&tid, NULL, proxy_handler, (void *)item);
        }
    }

    /* no thread will finish code before job is done */
    pthread_join(tid, NULL);
    return 0;
}
