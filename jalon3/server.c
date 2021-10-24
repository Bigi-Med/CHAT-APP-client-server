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
char salles_list[] = "Online salons are : \n";
char destroyed[] = "The following salon is destroyed : ";
char joined[] = "The following user has joined the salon : ";
char left[] = "The following user has left the sallon : ";
struct salon *head_salon = NULL;
struct connection_information *client_head = NULL;
int salons_left = 0;



struct connection_information {
	char * ip_address;
	int port_number;
	int socket_fd;
	char *pseudo;
	char *date;
	char *salon_name;
	struct connection_information *next;
};

struct salon{
	char *nom_salon;
	int Nutilisateurs;
	struct salon *next;
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
	//memset(client->salon_name,0,NICK_LEN);
	client->salon_name = NULL;
	client->next = NULL;
	return client;
}

struct salon *salon_head(int client_fd,char *sallon,int number)
{
	struct salon *salle = NULL;
	salle = (struct salon *)malloc(sizeof(struct salon));
	salle->nom_salon = sallon;
	salle->Nutilisateurs = number;
	return salle;
}

void add_sal_end(struct salon *salle,int client_fd,char *sallon,int number)
{
	struct salon *current = NULL;
	struct salon *tmp;
	current = (struct salon *)malloc(sizeof(struct salon));

	current->nom_salon = sallon;
	current->Nutilisateurs = number;
	tmp = salle;
	while(tmp !=NULL && tmp->next != NULL)
		tmp = tmp->next;
	
	tmp->next = current;
	
	return;
}

void are_there_salons(struct salon *salle)
{
	struct salon *p;
	p = salle;
	if(p == NULL)
	{
		salons_left++;
		return;
	}
	return;
}

void add_sal_to_client(int sockfd,struct connection_information *client,char *salle)
{
	struct connection_information *p;
	p = client;
	while(p!=NULL)
	{
		if(p->socket_fd == sockfd)
		{
			p->salon_name = salle;
			return;
		}
		p=p->next;
	}
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
	//memset(current->salon_name,0,NICK_LEN);
	current->salon_name = NULL;
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

void freeing_sallon(struct salon **salle,char *name)
{
	struct salon *head = *salle;
	struct salon *prev ;

	if(head !=NULL && !(strncmp(head->nom_salon,name,MIN(strlen(head->nom_salon),strlen(name)))))
		{
			*salle = head->next; // we keep the new head of linked list
			free(head);// we free the old head
			return;
		}	

		while(head !=NULL && (strncmp(head->nom_salon,name,MIN(strlen(head->nom_salon),strlen(name)))))
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

void user_join_sallon(int sockfd,char * name,struct connection_information *client)
{
	struct connection_information *p;
	p = client;

	while(p!=NULL)
	{
		if(p->socket_fd==sockfd)
		{
			p->salon_name = name;
			printf("sallon adde \n");
			return;
		}
		p=p->next;
	}
}

int multi_create(int sockfd,struct message msg,struct connection_information *client ,char *buff)
{
	static int first_salon = 0;
	//receiving message
	memset(buff,0,NICK_LEN);
	char *name_salon = malloc(NICK_LEN);
	memset(name_salon,0,NICK_LEN);
	strncpy(name_salon,msg.infos,strlen(msg.infos));
	
	/*if(recv(sockfd,buff,msg.pld_len,0)<0)
	{
		perror("Error while receiving multi message");
	}*/
	
	printf("salon name is %s \n",name_salon);
	//buff containes salon name
	//creating salon
	printf("SALON_LEFTT IS %i \n",salons_left);
	struct salon *p;
	p = head_salon;
	while(p!=NULL)
	{
		//if(!(strncmp(p->nom_salon,name_salon,MIN(strlen(p->nom_salon),strlen(name_salon)))))
		if(strcmp(p->nom_salon,name_salon)==0)
		{
			
			memset(&msg,0,sizeof(msg));
			msg.type = MULTICAST_CREATE;
			//send struct with MULTICAST CREATE
			if(send(sockfd,&msg,sizeof(msg),0)<0)
			{
				perror("Error while sending multi struct");
				
			}
			return 0;

		}
		p=p->next;
	}
	if(first_salon == 0 || salons_left == 0)
	{
		//struct salon *head_salon = NULL;
		printf("adding head \n");
		head_salon = salon_head(sockfd,name_salon,1);
		user_join_sallon(sockfd,name_salon,client);
		first_salon++;
		salons_left--;
		return 1;
	}
	else{
		printf("adding in end \n");
		add_sal_end(head_salon,sockfd,name_salon,1);
		user_join_sallon(sockfd,name_salon,client);
		
		
	}

	printf("sallon createed \n");
	return 1;
	
}
void multi_list(int sockfd,struct message msg,struct salon *salle,char *buff)
{
	memset(&msg,0,sizeof(msg));
	printf("before this memset \n");
	memset(buff,0,MSG_LEN);
	printf("after this memset \n");
	struct salon *p;
	p = salle;
	
	while(p!=NULL)
	{
		printf("in while loop \n");
		strcat(salles_list,p->nom_salon);
		strcat(salles_list,"\n");
		p = p->next;
	}

	//setting up struct

	memset(&msg,0,sizeof(msg));
	msg.type = MULTICAST_LIST;
	msg.pld_len = strlen(salles_list);
	strncpy(msg.nick_sender,"\0",1);
	strncpy(msg.infos,"\0",1);
	//sending the struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending who struct");
		return;
	}

	//sending who msg

	if(send(sockfd,salles_list,msg.pld_len,0)<0)
	{
		perror("Error wihle sendig who msg");
		return;
	}
	printf("msg sent \n");
	
	printf("%s \n",salles_list);
	memset(&msg,0,sizeof(msg));
	strcpy(salles_list,"Online salons are : \n");
}

void notify_users(struct connection_information *client,char *joiner_leaver,int leave_join,char *salon_nam)
{
	//leave_join is 1 if join and 0 if leave
	struct connection_information *p;
	struct message msgstruct;
	p = client;
	
	
	
	while(p!=NULL)
	{
	if(!(strncmp(joiner_leaver,p->pseudo,MIN(strlen(joiner_leaver),strlen(p->pseudo)))))
	{
		p = p->next;
		printf("first if\n");
		continue;
	}
	
	/*else if(strcmp(p->salon_name,salon_nam))
	{
		p = p->next;
		printf("second if \n");
		continue;
	}*/
	else{
		memset(&msgstruct,0,sizeof(msgstruct));
		strcat(joined,joiner_leaver);
		strcat(joined,"\n");
		strcat(left,joiner_leaver);
		strcat(left,"\n");
		msgstruct.type = ECHO_SEND;
		strncpy(msgstruct.infos,"\0",1);
		strncpy(msgstruct.nick_sender,"\0",1);
		if(leave_join == 1)
			msgstruct.pld_len = strlen(joined);
		else 
			msgstruct.pld_len = strlen(left);
		if(send(p->socket_fd,&msgstruct,sizeof(msgstruct),0)<0)
		{
			perror("Error while sending new joined struct ");
		}
		if(leave_join == 1)
		{
			if(send(p->socket_fd,joined,msgstruct.pld_len,0)<0)
			{
				perror("Error while sending new joined message");
			}
		}
		else
		{
			if(send(p->socket_fd,left,msgstruct.pld_len,0)<0)
			{
				perror("Error while sending new joined message");
			}
		}

	}
	p = p->next;
	strcpy(joined,"The following user has joined the salon : ");
	strcpy(left,"The following user has left the sallon : ");
	}
}

void multi_join(int sockfd,struct message msg,char *buff,struct connection_information *client,struct salon *salle)
{
	memset(buff,0,NICK_LEN);
	struct connection_information *p;
	p = client;
	struct salon *q;
	q = salle;

	while(p!=NULL)
	{
		if(p->socket_fd == sockfd)
		{
			p->salon_name = msg.infos;
			
			while(q!=NULL)
			{
				if(!(strncmp(q->nom_salon,msg.infos,MIN(strlen(q->nom_salon),strlen(msg.infos)))))
				{
					printf("in condition add 1 \n");
					q->Nutilisateurs++;
					printf("user added succ\n");
					notify_users(client_head,p->pseudo,1,q->nom_salon);
					break;
				}
				q = q->next;
			}
			break;
		}
		p = p->next;
	}


}

void multi_quit(int sockfd,struct message msg,char *buff,struct connection_information *client,struct salon *salle)
{
	memset(buff,0,NICK_LEN);
	struct connection_information *p;
	p = client;
	struct salon*q;
	q = salle;
	memset(buff,0,MSG_LEN);
	while(p!=NULL)
	{
		if(p->socket_fd == sockfd)
		{
			//memset(p->salon_name,0,NICK_LEN);
			notify_users(client_head,p->pseudo,0,q->nom_salon);
			p->salon_name = NULL;
			printf("quitted channel \n");
			while(q!=NULL)
			{
				if(!(strncmp(q->nom_salon,msg.infos,MIN(strlen(q->nom_salon),strlen(msg.infos)))))
				{	
					printf("in condition add 1 \n");
					q->Nutilisateurs--;
					//notify_users(client_head,p->pseudo,0,q->nom_salon);
					if(q->Nutilisateurs == 0)
					{
						//if there are no users
						freeing_sallon(&head_salon,msg.infos);
						are_there_salons(head_salon);
						//sending sallon is destroyed info
						
						strcat(destroyed,msg.infos);
						strcat(destroyed,"\n");
						memset(&msg,0,sizeof(msg));
						msg.pld_len = strlen(destroyed);
						
						strncpy(msg.infos,"\0",1);
						strncpy(msg.nick_sender,"\0",1);
						msg.type = MULTICAST_QUIT;
						//SENDING DESTROY STRUCT
						if(send(sockfd,&msg,sizeof(msg),0)<0)
						{
							perror("Error while sending destroy struct");
						}
						printf("struct destroy sent\n");
						//sending destroyed message
						if(send(sockfd,destroyed,msg.pld_len,0)<0)
						{
							perror("Error while sending destroy message");
						}
						strcpy(destroyed, "The following salon is destroyed : ");
						
					}
					
					break;
				}
				q = q->next;
			}
			break;
		}
	p = p->next;
	}
}

void multi_send(int sockfd,struct message msg,struct connection_information *client)
{

	struct connection_information *p;
	struct message msgstruct;
	p = client;
	msgstruct = msg;
	char buff[MSG_LEN];
	memset(buff,0,MSG_LEN);
	if(recv(sockfd,buff,msg.pld_len,0)<0)
	{
		perror("Error while receiving multi send strcut ");
	}
	//we now have the message in buff;
	//sending the message for other users;
	char the_salon[NICK_LEN];
	char sender[NICK_LEN];
	memset(sender,0,NICK_LEN);
	memset(the_salon,0,NICK_LEN);
	strncpy(the_salon,msg.infos,strlen(msg.infos));
	strncpy(sender,msg.nick_sender,strlen(msg.nick_sender));
	memset(&msgstruct,0,sizeof(msgstruct));
	msgstruct.type = MULTICAST_SEND;
	msgstruct.pld_len = strlen(buff);
	strncpy(msgstruct.infos,"\0",1);
	strncpy(msgstruct.nick_sender,sender,strlen(sender));
	printf("in while loop\n");
	while(p!=NULL)
	{
		if(strcmp(p->pseudo,sender)==0)
		{
			printf("in sender \n");
			p = p->next;
			continue;
		}
		else{
			if(p->salon_name == NULL)
			{	p=p->next;
				continue;
			}
			if(strcmp(p->salon_name,the_salon)==0)
			{
				//sending struct
				if(send(p->socket_fd,&msgstruct,sizeof(msgstruct),0)<0)
				{
					perror("Error while sending multi struct ");
				}
				printf("struct was sent \n");
				printf("message to send is : %s \n",buff);
				if(send(p->socket_fd,buff,msgstruct.pld_len,0)<0)
				{
					perror("Error while sending multi message");
				}
				printf("message multi sent \n");
				p=p->next;
				continue;
			}
			
			else{
				printf("not anywhere \n");
				p = p->next;
				continue;
			}
		}
	}




	/*int pp = 0;
	char salonn[NICK_LEN];
	char nickname[NICK_LEN];
	memset(nickname,0,NICK_LEN);
	memset(salonn,0,NICK_LEN);
	strncpy(salonn,msg.infos,strlen(msg.infos));
	strncpy(nickname,msg.nick_sender,strlen(msg.nick_sender));
	//memset(&msg,0,sizeof(msg ));
	struct connection_information *p;
	p = client;
	char buff[MSG_LEN];
	memset(buff,0,MSG_LEN);
	if(recv(sockfd,buff,msg.pld_len,0)<0)
	{
		perror("Error while recv multi send struct");
	}
	printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s , /msgis : %s\n", msg.pld_len, msg.nick_sender, msg_type_str[msg.type], msg.infos,buff);

	msg.type = MULTICAST_SEND;
	msg.pld_len = strlen(buff);
	strncpy(msg.infos,"\0",1);
	strncpy(msg.nick_sender,nickname,strlen(nickname));
	memset(&msg,0,sizeof(msg ));
	while(p->next!=NULL)
	{
		pp++;
		if(strcmp(p->salon_name,salonn)==0)
		{
			msg.type = MULTICAST_SEND;
			msg.pld_len = strlen(buff);
			strncpy(msg.infos,"\0",1);
			strncpy(msg.nick_sender,nickname,strlen(nickname));
			if(send(p->socket_fd,&msg,sizeof(msg),0)<0)
			{
				perror("Error while sending multi struct");
				break;
			}
			printf("MULTI SENT WAS \n");
			printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msg.pld_len, msg.nick_sender, msg_type_str[msg.type], msg.infos);
			if(send(p->socket_fd,buff,msg.pld_len,0)<0)
			{
				perror("Error while sending multi message");
				break;
			}
			printf("msg to send is %s \n",buff);
			//p = p->next;
			printf(" p is %i \n",pp);
			if(p == NULL);
			
			printf("p is NULL \n");
			//continue;
		}
		else{
			p=p->next;
			//continue;
		}
		p = p->next;
	}*/
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
	//struct connection_information *client_head = NULL;
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
				free(nick);
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
				if(msgstruct.type == MULTICAST_CREATE)
				{
					printf("in multi create \n");
					if(!multi_create(pollfds[i].fd,msgstruct,client_head,buff))
						continue;
				}
				if(msgstruct.type == MULTICAST_LIST)
				{
					printf("in mulit list\n");
					multi_list(pollfds[i].fd,msgstruct,head_salon,buff);
				}
				if(msgstruct.type == MULTICAST_JOIN)
				{
					printf("in join \n");
					multi_join(pollfds[i].fd,msgstruct,buff,client_head,head_salon);
				}
				if(msgstruct.type == MULTICAST_QUIT)
				{
					printf("in multi quit \n");
					multi_quit(pollfds[i].fd,msgstruct,buff,client_head,head_salon);
				}
				if(msgstruct.type == MULTICAST_SEND)
				{
					printf("in multi send \n");
					multi_send(pollfds[i].fd,msgstruct,client_head);
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
						free(nick);
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