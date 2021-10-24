Project: Socket Programming – HTTP Server and Proxy Serve 
In this project, you are required to implement a multi-threaded HTTP server that returns a document determined by the requested URI. You are also requested to implement a proxy server with some specific properties. You are also required to use ApacheBench program for testing your servers. 
Objectives of http server (all done)
	•	Your server should be capable of providing concurrency via multi-threading.
	•	Your server program should take single argument which specifies the port number.
	•	Your server should return an HTML document according to the requested URI. 
	•	Your server must send back an HTTP response line
	•	Your server should send back an HTTP response line that indicates errors
	•	Your server should print out information about every message received and every message sent.
	•	Your server should work when connected via a web browser 
Objectives of proxy server (all done)
	•	Your proxy server will not be able to cache HTTP objects. (extra 15 points) – done (video added)
	•	Proxy Server should use port 8888. (you can give any port number as an argument)
	•	Your proxy server only directs the requests to your web server. (extra 3 points) - done
	•	In this project, proxy server has a restriction of URL length 9999.
	•	If the Web Server is not running currently, your proxy server would return a “Not Found” error message with status code 404.
	•	The proxy server should work when connected via a browser after configuring the proxy settings of your browser. 



			
Implementing Multi-Threaded HTTP Server


Socket implementation
	•	Implementing socket connection is pretty straight forward, you can see the commented code below:

	•	    /* creating socket */
	•	
	•	    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
	•	   /* server is socket struct, filling its attiributes */
	•	
	•	    server.sin_family = AF_INET;
	•	    server.sin_addr.s_addr = INADDR_ANY;
	•	    server.sin_port = htons(PORT_NO);
	•	  /* bind socket with its id */
	•	
	•	    if (bind(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	•	    {
	•	        printf("Binding failed\n");
	•	        return 1;
	•	    }
	•	/* listens requests */
	•	
	•	    listen(sock_desc, 20);
	•	





Multithread implementation
	•	For each connection request from client, new thread with given arguments runs tcp_handler function which handles request & response cycle between client and the server.
while (
(new_socket = accept(sock_desc, (struct sockaddr*)&client, (socklen_t *)&c))
) 

// Accept the connection.
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







tcp_handler function
void *tcp_handler(void *sock_desc, void *arg)
socket id
int sock = *((int *)sock_desc);
get request
request = recv(sock, client_reply, BUFFER_SIZE, 0);

this function gets the number from URL
ex: GET /1500 /…. .. .. …    1500
int size = GET_SIZE(client_reply);
if size < 1 (This means there is no integer inside http request)
if(size < 1)
        {
            char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> URL must be a number </body></html>";
            write(sock, message, strlen(message));
            return NULL;
        }
Put control ( if request is PUT then “Not implemented” error will be sent) 
if (PUT_CONTROL(client_reply) > 0)
Post control ( if request is PUT then “Not implemented” error will be sent) 
if (POST_CONTROL(client_reply) > 0)
100 – 20000 control ( if requested URL is not sized between this to, then error will be sent)
if (size < 100 || 20000 < size)




Constructing the http Response msg if every error handling segment passed 



Send response to the client
write(sock, temp, strlen(temp));


free the allocated memories and sockets
    free(sock_desc);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = -1;
    return 0;
}





Proxy server implementation
	•	Proxy server implementation was more complex. Since in infinite loop there must be 3 connection at the same time. Also caching implemented this makes it more complex

	•	Cache implementation


This node holds response msg, and a flag to indicate its full or not.
struct cache_node
{
    char response_msg[MAX_SIZE];
    int full;
};




Create 20000 sized node array for 20000 different requests 1 to 20000
And initialize their flag to -1

struct cache_node *st = malloc(20000 * sizeof(struct cache_node));
    for (int i = 0; i < 20000; i++)
    {
        st[i].full = -1;
    }




Given as an argument to thread

struct cache_node * st = info->st;






Size is being index for our cache node array, if request is GET/ … /100 then we store response in 100. index of the array
int size = GET_SIZE(buffer);









When request is come, if the nodes full flag in the index size show is < 0 then the request will go to the server, and response will be stored in the index shown by size.

  if (st[size].full < 0)
    {
        strcpy(st[size].response_msg, buffer);
        st[size].full = 1;
        printf("REQUEST & RESPONSE CACHED \n");
        printf("\nSERVER RESPONSE IS: \n\n %s", st[size].response_msg);
    }






When request is come, this means response already stored in node array and no need to request it from server.  


    if (st[size].full > 0)
    {
        write(info->client_fd, st[size].response_msg, sizeof(st[size].response_msg));
        printf("CACHED RESPONSE USED \n\n SERVER RESPONSE IS: %s \n\n", st[size].response_msg);
        return NULL;
    }



















	•	Directing to any webserver implementation

For this we used C’s libraries, this functin converts a domain address to an ip address

ex: localhost to 127.0.0.1
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
    }    addr_list = (struct in_addr **)he->h_addr_list;
    for (i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }
    return 1;
}














	•	Proxy implementation



This is argument node for our thread, which has cache arrays pointer, clients socket id, ip address, hostname (for print issues), and servers port


struct thread_arg
{

    struct cache_node *st;
    int client_fd;
    char ip[100];
    char port[100];
    char hostname[100];
};



Inside main, we connecting proxy server with  client

proxy_fd = socket(AF_INET, SOCK_STREAM, 0)
proxy_sd.sin_family = AF_INET;
    proxy_sd.sin_port = htons(atoi(proxy_port));
    proxy_sd.sin_addr.s_addr = INADDR_ANY;

bind(proxy_fd, (struct sockaddr *)&proxy_sd, sizeof(proxy_sd)
listen(proxy_fd, SOMAXCONN)



when a connection request is come

client_fd = accept(proxy_fd, (struct sockaddr *)&client, (socklen_t *)&c);






we fill our thread argumans and invoke thread with function proxy_handler


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


proxy_handler function

void *proxy_handler(void *arg)

construct server socket connection between proxy (client – proxy already done)
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



URL construct



URL length check
/* URL length check */

    if (strlen(URL) > 9999)
    {
        char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> URL IS TOO LONG </body></html>";
        write(info->client_fd, message, sizeof(message));
        return NULL;
    }

Server down check
   if (check_read <= 0)
    {
        char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> Server is down. </body></html>";
        write(info->client_fd, message, sizeof(message));
        return NULL;
    }












Proxy send – request cycle 

Get request from client

check_read = read(info->client_fd, buffer, sizeof(buffer));


Send it to the server 

write(server_fd, buffer, sizeof(buffer));



Receive response from server

check_read = read(server_fd, buffer, sizeof(buffer));


Send response to client

write(info->client_fd, buffer, sizeof(buffer));




























Multithread Server Run Time Screenshots

localhost:5000/10


localhost:5000/100

localhost:5000/20001

localhost:5000/a
























Proxy Server Run Time Screenshots


Server 3000


Proxy 3000 8887



localhost:8888/50


localhost:8888/a


localhost:8888/150










Apache – Bench reports

ab -n 1000 -c 1 http://127.0.0.1:500/

Since its working on localhost, its too fast to measure. 



ab -n 1000 -c 5 http://127.0.0.1:500/




ab -n 1000 -c 10 http://127.0.0.1:500/

since concurrency level increase, delay increases.


ab -n 1000 -c 1 -k http://127.0.0.1:500/


ab -n 1000 -c 5 -k http://127.0.0.1:500/

-k increases delay since every request tries too keep connection alive.











ab -n 1000 -c 10 -k http://127.0.0.1:500/


