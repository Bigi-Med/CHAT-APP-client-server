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
#include <time.h>

#include "common.h"
#include "msg_struct.h"

#define BACKLOG 20
#define MIN(x,y) ((x)<(y)?(x):(y))

char nick_ask[] = "[Server] : please login with /nick <your pseudo>";
char welcome[] = "[Server] : Welcome on the server ";
char used[] = "Pseudo already used";
char user_list[] = "[SERVER] : Online users are : ";
char user_1[] = "[SERVER] : ";
char user_2[] = " connected since :";
char user_3[] = " with IP adresse : ";
char user_4[] = " on port number :";



struct connection_information {
	char * ip_address;
	int port_number;
	int socket_fd;
	char *pseudo;
	char *date;
	struct connection_information *next;
};

struct connection_information * chained_list(int client_fd,char * ip,int port,char *pseudo_name,char *timer)
{
	//creating the head of the linked list
	struct connection_information *client = NULL;
	client = (struct connection_information *) malloc(sizeof(struct connection_information));
	client->ip_address = ip;
	client->port_number = port;
	client->pseudo = pseudo_name;
	client->socket_fd = client_fd;
	client->date = timer;
	client->next = NULL;
	return client;
}

struct connection_information * add_end(struct connection_information *client,int client_fd,char *ip, int port,char *pseudo_name,char *timer)
{
	// adding the new structure to the end of the linked list
	struct connection_information *current = NULL;
	struct connection_information *tmp;
	current = (struct connection_information *)malloc(sizeof(struct connection_information));
	

	
	
	current->ip_address = ip;
	current->port_number = port;
	current->pseudo = pseudo_name;
	current->socket_fd = client_fd;
	current->date = timer;
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

int is_used(char *pseudo,struct connection_information *client )//return 1 if not used , 0 otherwise
{
	
	while(client!=NULL)
	{
		if(!strcmp(client->pseudo,pseudo))
		{
			
			return 0;
		}
		client = client->next;
	}
	return 1;
}

void set_new_nick(struct connection_information *p,struct message msg,int sock)
{
	
		struct connection_information *q;
		q = p;
		while(q!=NULL)
		{
		if(p->socket_fd == sock)
		{
			strcpy(p->pseudo,msg.nick_sender);
			break;
		}
		p=p->next;
	}
}
void nick_list(struct connection_information *q,struct message msg,int sock,char *buff)
{
	memset(&msg,0,sizeof(msg));
	memset(buff,0,MSG_LEN);
	struct connection_information *p;
	p = q;
	
	while(p!=NULL)
	{
		printf("in while loop \n");
		strcat(user_list,p->pseudo);
		strcat(user_list,"\n");
		p = p->next;
	}

	//setting up struct

	memset(&msg,0,sizeof(msg));
	msg.type = NICKNAME_LIST;
	msg.pld_len = strlen(user_list);
	strncpy(msg.nick_sender,"\0",1);
	strncpy(msg.infos,"\0",1);
	//sending the struct
	if(send(sock,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending who struct");
		return;
	}

	//sending who msg

	if(send(sock,user_list,msg.pld_len,0)<0)
	{
		perror("Error wihle sendig who msg");
		return;
	}
	printf("msg sent \n");
	
	printf("%s \n",user_list);
	memset(&msg,0,sizeof(msg));
	strcpy(user_list,"[SERVER] : Online users are : ");
}

void nick_infos(struct connection_information *p,struct message msg,int sockfd)
{
	struct connection_information *q;
	q = p;
	
	while(q!=NULL)
	{
		if(!(strncmp(q->pseudo,msg.infos,strlen(q->pseudo))) )
		{
			strcat(user_1,q->pseudo);
			strcat(user_1,user_2);
			strcat(user_1,q->date);
			strcat(user_1,user_3);
			strcat(user_1,q->ip_address);
			strcat(user_1,"\n");
			break;
	}
		q = q->next;
	}

	//setting up the struct

	msg.pld_len = strlen(user_1);
	msg.type = NICKNAME_INFOS;
	strncpy(msg.nick_sender,"\0",1);
	strncpy(msg.infos,"\0",1);
	//sending the struct

	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending whois struct");
		return;
	}

	//sending whois message

	if(send(sockfd,user_1,msg.pld_len,0)<0)
	{
		perror("Error while sendig whois msg");
		return;
	}

	memset(&msg,0,sizeof(msg));
	strcpy(user_1,"[SERVER] : ");
	strcpy(user_2, " connected since :");
	strcpy(user_3 , " with IP adresse : ");
}

void broad_send(struct connection_information *p,struct message msg,int sockfd,char *buff)
{
	memset(buff,0,MSG_LEN);

	if(recv(sockfd,buff,msg.pld_len,0)<0)
	{
		perror("Error while receiving broadcast message");
		return;
	}
	//sending message 
	//looking for active sockets to send to
	struct connection_information *active;
	active = p;
	while(active!=NULL)
	{
		if(active->socket_fd == sockfd)
		{
			active = active->next;
			continue;
		}
		else{
			//send struct
			if(send(active->socket_fd,&msg,sizeof(msg),0)<0)
			{
				perror("Error while sending br struct");
				break;
			}
			//send msg
			if(send(active->socket_fd,buff,msg.pld_len,0)<0)
			{
				perror("Error while sending br msg");
				break;
			}
			
		}
		active = active->next;
	}
}
void uni_send(struct connection_information *q,struct message msg,int sockfd,char *buff)
{
	//receiving message
	if(recv(sockfd,buff,msg.pld_len,0)<0)
	{
		perror("Error while receiving unicast msg");
		return;
	}
	//searching for destinated client
	struct connection_information *p;
	p = q;
	while(p!=NULL)
	{
		if(!(strncmp(p->pseudo,msg.infos,MIN(strlen(p->pseudo),strlen(msg.infos)))))
		{
			
			//send struct 
			if(send(p->socket_fd,&msg,sizeof(msg),0)<0)
			{
				perror("Error while sending unicast struct");
				break;
			}
			//send msg
			if(send(p->socket_fd,buff,msg.pld_len,0)<0)
			{
				perror("Error while sending unicast message");
				break;
			}
			printf("msg sent to %s and is %s \n",p->pseudo,buff);
			break;
		}
		
		p = p->next;
	}
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
			/*printf("Binding to %s on port %hd\n",
						 inet_ntoa(local_address),
						 ntohs(sockptr->sin_port));*/

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
	struct connection_information *client_head = NULL;
		struct connection_information *current ;
		static int cach = 0;
		static int mem = 0;
		struct message msgstruct;
	// Declare array of struct pollfd
	int nfds = 10;
	struct pollfd pollfds[nfds];
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

		char *nick = malloc(NICK_LEN);
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
				//asking client to insert pseudo
				//setting up structure
				msgstruct.type = NICKNAME_NEW;
				strncpy(msgstruct.infos,"\0",1);
				strncpy(msgstruct.nick_sender,"\0",1);
				//char full_msg[MSG_LEN];
				msgstruct.pld_len = strlen(nick_ask);
				//sending struct
				if(send(client_fd,&msgstruct,sizeof(msgstruct),0)<0)
				{
					perror("Error while sending nick struct");
					break;
				}
				//sending message
				if(send(client_fd,nick_ask,msgstruct.pld_len,0)<0)
				{
					perror("Error while sending nick message");
					break;
				}
					/*************************************/
					memset(&msgstruct,0,sizeof(msgstruct));
				
				while(1)
				{
					memset(&msgstruct,0,sizeof(msgstruct));
				if(recv(client_fd,&msgstruct,sizeof(msgstruct),0)<0)
				{
					perror("Error while receiving structure");
					break;
				}
				printf("received nick struct \n");
				//if the struct is of type nick new, send welcome message
				if(strcmp(msg_type_str[msgstruct.type] ,"NICKNAME_NEW")==0)
				{
					if(is_used(msgstruct.infos,client_head) == 0)
					{
						//we send a struct with nick_sender = "\0"
						memset(&msgstruct,0,sizeof(msgstruct));
						msgstruct.pld_len = 0;
						strncpy(msgstruct.infos,"\0",1);
						strncpy(msgstruct.nick_sender,"\0",1);
						msgstruct.type  = NICKNAME_NEW;
						if(send(client_fd,&msgstruct,sizeof(msgstruct),0)<0)
						{
							perror("Error while sending again nick");
						}
						continue;
					}
					else{
					printf("%s \n",welcome);
					strcat(welcome,msgstruct.infos);
					printf("infos %s \n",msgstruct.infos);
					printf("%s \n",welcome);
					strcpy(nick ,msgstruct.infos);
					
					//nick = msgstruct.infos;
					//setting up the weclome structure
					memset(&msgstruct,0,sizeof(msgstruct));
					msgstruct.type = ECHO_SEND;
					strncpy(msgstruct.nick_sender,nick,strlen(nick));
					strncpy(msgstruct.infos,"\0",1);
					msgstruct.pld_len = strlen(welcome);
					//sending the struct
					if(send(client_fd,&msgstruct,sizeof(msgstruct),0)<0)
					{
						perror("Error while sending welcome struct");
						break;
					}
					//sending the message
					if(send(client_fd,welcome,msgstruct.pld_len,0)<0)
					{
						perror("Error while sendig welcome message");
						break;
					}
					 memset(&msgstruct,0,sizeof(msgstruct));
					strcpy(welcome,"[Server] : Welcome on the server ");
					printf("%s \n",welcome);
					break;
					}
				}
				}
				/************************/
				if (client_fd == 4 && cach == 0)
				{	
					cach++;
					time_t t;
					time(&t);
					client_head = chained_list(client_fd,inet_ntoa(client_address),ntohs(sockptr->sin_port),nick,ctime(&t));
				}
				
				if ((client_fd > 4)|| (cach >= 2 && client_fd == 4))
				{
					cach++;
					time_t t;
					time(&t);
					current = add_end(client_head,client_fd,inet_ntoa(client_address),ntohs(sockptr->sin_port),nick,ctime(&t));
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
				memset(&msgstruct,0,sizeof(msgstruct));
				struct connection_information *p;
				p = client_head;
				while(p!=NULL)
				{
					printf("socket %d nickname %s \n",p->socket_fd,p->pseudo);
					p = p->next;
				}
				//receiving structure
				if(recv(pollfds[i].fd,&msgstruct,sizeof(msgstruct),0)<0)
				{
					perror("Error while receiving structure");
					break;
				}
				printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msgstruct.pld_len, msgstruct.nick_sender, msg_type_str[msgstruct.type], msgstruct.infos);
				if(msgstruct.type == NICKNAME_NEW)
				{
						set_new_nick(client_head,msgstruct,pollfds[i].fd);
				}
				if(msgstruct.type == NICKNAME_LIST)
				{
					nick_list(client_head,msgstruct,pollfds[i].fd,buff);
				}
				if(msgstruct.type == NICKNAME_INFOS)
				{
					nick_infos(client_head,msgstruct,pollfds[i].fd);
				}
				if(msgstruct.type == BROADCAST_SEND)
				{
					broad_send(client_head,msgstruct,pollfds[i].fd,buff);
				}
				if(msgstruct.type == UNICAST_SEND)
				{
					uni_send(client_head,msgstruct,pollfds[i].fd,buff);
				}
				//receiving message
				if(strcmp(msg_type_str[msgstruct.type] ,"ECHO_SEND")==0){
					memset(buff,0,MSG_LEN);
				if (recv(pollfds[i].fd, buff, MSG_LEN, 0) < 0) {
					perror("Error while receiving message");
					break;
				}
				//send a struct 
				if(send(pollfds[i].fd,&msgstruct,sizeof(msgstruct),0)<0)
				{
					perror("Error while sending a struct");
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
				}
				pollfds[i].revents = 0;
			}
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