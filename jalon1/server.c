#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

#define BACKLOG 20





struct connection_information {
	char * ip_address;
	int port_number;
	int socket_fd;
	struct connection_information *next;
};

struct connection_information * chained_list(int client_fd,char * ip,int port)
{
	//creating the head of the linked list
	struct connection_information *client = NULL;
	client = (struct connection_information *) malloc(sizeof(struct connection_information));
	client->ip_address = ip;
	client->port_number = port;
	client->socket_fd = client_fd;
	client->next = NULL;
	return client;
}

struct connection_information * add_end(struct connection_information *client,int client_fd,char *ip, int port)
{
	// adding the new structure to the end of the linked list
	struct connection_information *current = NULL;
	struct connection_information *tmp;
	current = (struct connection_information *)malloc(sizeof(struct connection_information));
	

	
	
	current->ip_address = ip;
	current->port_number = port;
	current->socket_fd = client_fd;
	current->next = NULL;
	tmp = client;
	while(tmp !=NULL && tmp->next != NULL)
		tmp = tmp->next;
	
	tmp->next = current;
	
	return current;
	
}

void freeing(struct connection_information **client,int fd)
{
	
	struct connection_information *head = *client;
	struct connection_information *prev ;

	if(head !=NULL && head->socket_fd == fd)
		{
			*client = head->next; // we keep the new head of linked list
			free(head);// we free the old head
			return;
		}	

		while(head !=NULL && head->socket_fd != fd)
		{
			//we look for the struct with the wanted fd to free
			prev = head;
			head = head->next;
			//the freed structure is head, the previous one is prv
		}
		prev->next = head->next;//we tie the prv structure to the one after head

		free(head);//deleting the wanted structure
	
}

int read_from_socket(int fd, void *buf, int size) {
	int ret = 0;
	int offset = 0;
	while (offset != size) {
		ret = read(fd, buf + offset, size - offset);
		if (-1 == ret) {
			perror("Reading from client socket");
			exit(EXIT_FAILURE);
		}
		if (0 == ret) {
			printf("Should close connection, read 0 bytes\n");
			close(fd);
			return -1;
			break;
		}
		
		offset += ret;
	}
	return offset;
}

int write_in_socket(int fd, void *buf, int size) {
	int ret = 0, offset = 0;
	while (offset != size) {
		if (-1 == (ret = write(fd, buf + offset, size - offset))) {
			perror("Writing from client socket");
			return -1;
		}
		offset += ret;
	}
	return offset;
}


int socket_listen_and_bind(char *port) {
	int listen_fd = -1;
	if (-1 == (listen_fd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("Socket");
		exit(EXIT_FAILURE);
	}
	printf("Listen socket descriptor %d\n", listen_fd);

	int yes = 1;
	if (-1 == setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	struct addrinfo indices;
	memset(&indices, 0, sizeof(struct addrinfo));
	indices.ai_family = AF_INET;
	indices.ai_socktype = SOCK_STREAM;
	indices.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	struct addrinfo *res, *tmp;

	int err = 0;
	if (0 != (err = getaddrinfo(NULL, port, &indices, &res))) {
		errx(1, "%s", gai_strerror(err));
	}

	tmp = res;
	while (tmp != NULL) {
		if (tmp->ai_family == AF_INET) {
			struct sockaddr_in *sockptr = (struct sockaddr_in *)(tmp->ai_addr);
			struct in_addr local_address = sockptr->sin_addr;
			printf("Binding to %s on port %hd\n",
						 inet_ntoa(local_address),
						 ntohs(sockptr->sin_port));

			if (-1 == bind(listen_fd, tmp->ai_addr, tmp->ai_addrlen)) {
				perror("Binding");
			}
			if (-1 == listen(listen_fd, BACKLOG)) {
				perror("Listen");
			}
			freeaddrinfo(res);
			return listen_fd;
		}
		tmp = tmp->ai_next;
	}
	freeaddrinfo(res);
	return listen_fd;
}

int server(int listen_fd) {

		struct connection_information *client_head ;

				
		struct connection_information *current ;
		static int cach = 0;
		static int mem = 0;
	

	// Declare array of struct pollfd
	int nfds = 10;
	struct pollfd pollfds[nfds];
	//struct connection_information *info = malloc(sizeof(struct connection_information)) ;
	// Init first slot with listening socket
	pollfds[0].fd = listen_fd;
	pollfds[0].events = POLLIN;
	pollfds[0].revents = 0;
	// Init remaining slot to default values
	for (int i = 1; i < nfds; i++) {
		pollfds[i].fd = -1;
		pollfds[i].events = 0;
		pollfds[i].revents = 0;
	}

	// server loop
	while (1) {

		// Block until new activity detected on existing socket
		int n_active = 0;
		if (-1 == (n_active = poll(pollfds, nfds, -1))) {
			perror("Poll");
		}
		printf("[SERVER] : %d active socket\n", n_active);

		// Iterate on the array of monitored struct pollfd
		for (int i = 0; i < nfds; i++) {

			// If listening socket is active => accept new incoming connection
			if (pollfds[i].fd == listen_fd && pollfds[i].revents & POLLIN) {
				
				

				// accept new connection and retrieve new socket file descriptor
				struct sockaddr client_addr;
				socklen_t size = sizeof(client_addr);
				int client_fd;
				if (-1 == (client_fd = accept(listen_fd, &client_addr, &size))) {
					perror("Accept");
					exit(EXIT_FAILURE);
				}
				// display client connection information
				struct sockaddr_in *sockptr = (struct sockaddr_in *)(&client_addr);
				struct in_addr client_address = sockptr->sin_addr;
				printf("Connection succeeded and client used %s:%hu \n", inet_ntoa(client_address), ntohs(sockptr->sin_port));
				printf("client_fd = %d\n", client_fd);
				
				if (client_fd == 4 && cach == 0)
				{	
					cach++;
					//printf("cach is %d \n",cach);
					client_head = chained_list(client_fd,inet_ntoa(client_address),ntohs(sockptr->sin_port));
					
				}

				if ((client_fd > 4)|| (cach >= 2 && client_fd == 4))
				{
					//printf("cach 2 is %d \n",cach);
					cach++;
					current = add_end(client_head,client_fd,inet_ntoa(client_address),ntohs(sockptr->sin_port));
				}
				// store new file descriptor in available slot in the array of struct pollfd set .events to POLLIN
				for (int j = 0; j < nfds; j++) {
					if (pollfds[j].fd == -1) {
						pollfds[j].fd = client_fd;
						pollfds[j].events = POLLIN;
						break;
					}
				}

				// Set .revents of listening socket back to default
				pollfds[i].revents = 0;

			} else if (pollfds[i].fd != listen_fd && pollfds[i].revents & POLLHUP) { // If a socket previously created to communicate with a client detects a disconnection from the client side
				// display message on terminal
				printf("client on socket %d has disconnected from server\n", pollfds[i].fd);
				// Close socket and set struct pollfd back to default
				close(pollfds[i].fd);
				freeing(&client_head,pollfds[i].fd);
				pollfds[i].events = 0;
				pollfds[i].revents = 0;
			} else if (pollfds[i].fd != listen_fd && pollfds[i].revents & POLLIN) { // If a socket different from the listening socket is active
			
				
				char buff[MSG_LEN];
				memset(buff,0,MSG_LEN);

				if (recv(pollfds[i].fd, buff, MSG_LEN, 0) <= 0) {
					break;
				}
				
				if(strcmp(buff,"/quit\n") == 0)
					{
						freeing(&client_head,pollfds[i].fd);
						close(pollfds[i].fd);
						int sock = pollfds[i].fd;
						pollfds[i].fd = -1;
						pollfds[i].events = 0;
						pollfds[i].revents = 0;
						printf("connection closed  in socket %i \n",sock);
						for(int k = 0; k<nfds;k++)
						{
							//for loop is used to check if all clients have disconnected, if so we set cach to 0 as it was at first, so that next client info will be stored at the head of linked list
							if (pollfds[k].fd == listen_fd || pollfds[k].fd == -1)
								{	
									mem++;
									continue;
								}
								else{
									mem = 0;//so that mem does not surpass nfds, if only one of many clients disconnected
									break;
								}
								
							}
							if( mem == nfds )//if the condition is true, then no clients is on, we thus set cach to 0 here 
							{
								cach = 0;
								mem = 0;
							}
						
						break;
					}
				
				printf("Received: %s \n", buff);

				if (send(pollfds[i].fd, buff, MSG_LEN, 0) <= 0) {
					break;
				}
				printf("Message sent!\n");

				
				// Set activity detected back to default
				pollfds[i].revents = 0;
			}
			//free(info);
			

		}
		
		
	}
}

int main(int argc, char  *argv[]) {

	// Test argc
	if (argc != 2) {
		printf("Usage: ./server port_number\n");
		exit(EXIT_FAILURE);
	}

	// Create listening socket
	char *port = argv[1];
	int listen_fd = -1;
	if (-1 == (listen_fd = socket_listen_and_bind(port))) {
		printf("Could not create, bind and listen properly\n");
		return 1;
	}
	// Handle new connections and existing ones
	server(listen_fd);

	return 0;
}
