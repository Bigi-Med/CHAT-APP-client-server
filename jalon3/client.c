#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include "common.h"
#include "msg_struct.h"

#define STR_MAX 128
#define FDS 2

char nick[NICK_LEN];
char salon[NICK_LEN];

struct message nick_new(int sockfd,struct message msg,char *buff)
{
	char new_nick[NICK_LEN];
	memset(new_nick,0,NICK_LEN);
	memcpy(new_nick,buff+6,strlen(buff)-7);
	//filling struct
	strncpy(msg.nick_sender,new_nick,strlen(new_nick));
	memset(nick,0,NICK_LEN);
	strncpy(nick,new_nick,strlen(new_nick));
	printf("You'r new nickname is %s \n",new_nick);
	strncpy(msg.infos,"\0",1);
	msg.type = NICKNAME_NEW;
	msg.pld_len = strlen(new_nick);
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending strucure");
		//break;
	}
	
	return msg;

}

struct message who_func(int sockfd,struct message msg,char *buff)
{
	memset(&msg,0,sizeof(msg));
	//setting up structure
	msg.type = NICKNAME_LIST;
	strncpy(msg.infos,"\0",1);
	strncpy(msg.nick_sender,nick,strlen(nick));
	msg.pld_len = 0;
//sending the structure
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending who sruct");
		//return;
	}
	printf("Message sent \n");
	return msg;
}

struct message whois_func(int sockfd,struct message msg,char *buff)
{
	memset(&msg,0,sizeof(msg));
	//retreiving target
	char target[NICK_LEN];
	memset(target,0,NICK_LEN);
	strncpy(target,buff+7,strlen(buff)-8);
	msg.type = NICKNAME_INFOS;
	strncpy(msg.infos,target,strlen(target));
	msg.pld_len = 0;
	strncpy(msg.nick_sender,nick,strlen(nick));
	//sendig whois structure
	if(send(sockfd,&msg,sizeof(msg),0)<0)
		{
			perror("Error while sending whois struct");
			//break;
		} 
		return msg;
}

struct message msgall_func(int sockfd,struct message msg, char *buff)
{
	char my_msg[MSG_LEN];
	memset(my_msg,0,MSG_LEN);
	memset(&msg,0,sizeof(msg));
	//retreiving the message
	strncpy(my_msg,buff+8,strlen(buff)-9);
	//setting up the struct
	msg.type = BROADCAST_SEND;
	strncpy(msg.nick_sender,nick,sizeof(nick));
	strncpy(msg.infos,"\0",1);
	msg.pld_len = strlen(my_msg);
	//sending the struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
		{
			perror("Error while sending broadcast struct");
			//break;
		}
		//sending message
	if(send(sockfd,my_msg,strlen(my_msg),0)<0)
		{
			perror("Error while sending broadcast message");
			//break;
		}
	
	return msg;
	
}

void msg_unicast(int sockfd,struct message msg,char buff[])
{
	char target[NICK_LEN];
	char msg_to_send[MSG_LEN];
	memset(msg_to_send,0,MSG_LEN);
	memset(target,0,NICK_LEN);
	int nick_length = 0;
	int msg_length = 0;
	int length = (int)strlen(buff);
	int i =5;
	while(i < length)
	{
		if(buff[i] == ' ')
		{	
			break;
		}
		nick_length++;
		i++;
	}
	int j = nick_length+6;
	while(j<length)
	{
		if(buff[j] == '\n')
			break;
		msg_length++;
		j++;
	}
	strncpy(target,buff+5,strlen(buff)-6-msg_length);
	strncpy(msg_to_send,buff+6+nick_length,strlen(buff)-6-nick_length-1);
	//setting up the struct to send
	memset(&msg,0,sizeof(msg));
	msg.type = UNICAST_SEND;
	strncpy(msg.infos,target,strlen(target));
	strncpy(msg.nick_sender,nick,strlen(nick));
	msg.pld_len = strlen(msg_to_send);
	//sending the struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending unicast struct");
		return;
	}
	//sending the message
	if(send(sockfd,msg_to_send,msg.pld_len,0)<0)
	{
		perror("Error while sending unicast message");
		return;
	}
	memset(buff,0,MSG_LEN);
	memset(&msg,0,sizeof(msg));

	return;
}
void ask_for_nick(int sockfd, struct message msg)
{
	char buff[MSG_LEN];
	memset(buff,0,MSG_LEN);
	int k;
	k=0;
	fflush(stdout);
	printf("in fucnt \n");
	printf("buff %s \n",buff);
	while ((buff[k++] = getchar()) != '\n') {}
	int check_nick =0;
	
	while(strncmp(buff,"/nick ",6)!=0)
	{
		printf("Please enter a pseudo \n");
		memset(buff,0,MSG_LEN);
		k = 0;
		while ((buff[k++] = getchar()) != '\n') {}
		
	
	}
	printf("buff is %s \n",buff);
	//retreiving the nick name

	//char nick[NICK_LEN];
	memset(nick,0,NICK_LEN);
	memcpy(nick,buff +6,strlen(buff)-7);
	memset(buff,0,MSG_LEN);
	printf("nick is %s \n",nick);
	memset(&msg,0,sizeof(msg));
	strncpy(msg.nick_sender,"\0",1);
	msg.type = NICKNAME_NEW;
	strncpy(msg.infos,nick,strlen(nick));
	msg.pld_len = strlen(msg.infos);

	//sending the struct containing the nick name
	printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msg.pld_len, msg.nick_sender, msg_type_str[msg.type], msg.infos);
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		printf("Error while sending nick new struct");
		return;
	}
	return;
}

void create_salon(int sockfd,struct message msg,char *buff)
{
	memset(&msg,0,sizeof(struct message));
	//char salon_name[NICK_LEN];
	//memset(salon_name,0,NICK_LEN);
	strncpy(msg.infos,buff+8,strlen(buff)-9);
	memset(salon,0,NICK_LEN);
	strncpy(salon,msg.infos,strlen(msg.infos));
	//setting up struct
	msg.pld_len = 0;
	strncpy(msg.nick_sender,nick,strlen(nick));
	msg.type = MULTICAST_CREATE;

	//sending struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending multi create struct");
	}
	//printf("MULTI SENT \n");

}

void ask_channel_list(int sockfd,struct message msg,char *buff)
{
	memset(&msg,0,sizeof(struct message));
	msg.pld_len = 0;
	strncpy(msg.nick_sender,nick,strlen(nick));
	strncpy(msg.infos,"\0",1);
	msg.type = MULTICAST_LIST;
	//SENDING STRUCT
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending multi list struct ");

	}

}

void join_sallon(int sockfd,struct message msg, char *buff)
{
	memset(&msg,0,sizeof(struct message));
	//char salon_name[NICK_LEN];
	//memset(salon_name,0,NICK_LEN);
	strncpy(msg.infos,buff+6,strlen(buff)-7);
	memset(salon,0,NICK_LEN);
	strncpy(salon,msg.infos,strlen(msg.infos));
	//setting up struct
	msg.pld_len = 0;
	strncpy(msg.nick_sender,nick,strlen(nick));
	msg.type = MULTICAST_JOIN;

	//sending struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending multi create struct");
	}
	printf("JOIN SENT \n");
}

void quit_sallon(int sockfd,struct message msg,char *buff)
{
	memset(&msg,0,sizeof(struct message));
	strncpy(msg.infos,buff+6,strlen(buff)-7);
	msg.pld_len = 0;
	strncpy(msg.nick_sender,nick,strlen(nick));
	msg.type = MULTICAST_QUIT;
	memset(salon,0,NICK_LEN);

	//sending struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending multi create struct");
	}
	printf("QUIT SENT \n");
	
}
struct message send_echo(int sockfd,struct message msg,char *buff)
{
	memset(&msg,0,sizeof(msg));
	msg.type = ECHO_SEND;
	strncpy(msg.nick_sender,nick,strlen(nick));
	strncpy(msg.infos,"\0",1);
	msg.pld_len = strlen(buff);
		//sending struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending strucure");
		//break;
	}
			
	// Sending message (ECHO)
	if (send(sockfd, buff, strlen(buff), 0) < 0) {
		perror("Error while sending message");
		//break;
	}
	printf("Message sent!\n");
	// Cleaning memory
	
		return msg;
}

struct message send_multicast(int sockfd,char *buff)
{
	struct message msg;
	memset(&msg,0,sizeof(msg));
	msg.type = MULTICAST_SEND;
	strncpy(msg.nick_sender,nick,strlen(nick));
	strncpy(msg.infos,salon,strlen(salon));
	msg.pld_len = strlen(buff);
		//sending struct
	if(send(sockfd,&msg,sizeof(msg),0)<0)
	{
		perror("Error while sending strucure");
		//break;
	}
	printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msg.pld_len, msg.nick_sender, msg_type_str[msg.type], msg.infos);	
	// Sending message (ECHO)
	if (send(sockfd, buff, msg.pld_len, 0) < 0) {
		perror("Error while sending message");
		//break;
	}

	printf("Message sent!\n");
	// Cleaning memory
	
		return msg;
}


void echo_client(int sockfd) {
	static int in_sallon = 0;
	//int checkk = 0;
	char buff[MSG_LEN];
	int is_nick = 0;
	struct pollfd pollfds[FDS];
	/*******/
	pollfds[0].fd = STDIN_FILENO;
	pollfds[0].events = POLLIN;
	pollfds[0].revents = 0;
	/********/
	pollfds[1].fd = sockfd;
	pollfds[1].events = POLLIN;
	pollfds[1].revents = 0;
	/*********/
	struct message msgstruct;
	static int check = 0;
	int n;
	memset(buff,0,MSG_LEN);
	memset(&msgstruct,0,sizeof(msgstruct));
	//receiving nick struct
	if (recv(sockfd,&msgstruct,sizeof(msgstruct),0)<0)
	{
		perror("Error while receiving nick struct");
		return;
	}
	//receiving nick msg
	if(recv(sockfd,buff,msgstruct.pld_len,0)<0)
	{
		perror("Error while receving nick msg");
		return;
	}
	printf("%s \n",buff);

	//treating the nickname
	while(1)
	{
		printf("in nick loop \n");
		ask_for_nick(pollfds[1].fd,msgstruct);
		memset(&msgstruct,0,sizeof(msgstruct));
		memset(buff,0,MSG_LEN);
		//receiving nick response struct

		if(recv(pollfds[1].fd,&msgstruct,sizeof(msgstruct),0)<0)
		{
			perror("Error while receiving nick response struct");
		}
		printf("received nick struct \n");
		if(!strncmp(msgstruct.nick_sender,"\0",1))
		{
			printf("in wrond cod\n");
			printf("Pseudo already taken, enter a new one \n");
			continue;
		}
		else{
			//receive welcome message and leave loop
			printf("in correct cond \n");
			if(recv(pollfds[1].fd,buff,msgstruct.pld_len,0)<0)
			{
				perror("Error while receiving welcome struct");
			}
			printf("%s \n",buff);
			break;
		}
	}

	
	while (1) {
		
		memset(buff, 0, MSG_LEN);
		memset(&msgstruct,0,sizeof(msgstruct));
		

		if (poll(pollfds, FDS, -1) <= 0){
			perror("Poll");
			break;
		}
		
		// Getting message from client
		if(pollfds[0].revents & POLLIN)
		{
		//printf("Message: ");
		memset(buff, 0, MSG_LEN);
		memset(&msgstruct,0,sizeof(msgstruct));
		n = 0;
		
		while ((buff[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
		
		//settign up  struct
		if(!(strncmp(buff,"/nick ",6)))
		{
			
			msgstruct = nick_new(pollfds[1].fd,msgstruct,buff);
			printf("Message sent!\n");
			 //cleaning memory
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}

		else if(!(strncmp(buff,"/who",4))&& (strlen(buff) == 5))
		{
			msgstruct = who_func(pollfds[1].fd,msgstruct,buff);
			
			memset(&msgstruct,0,sizeof(msgstruct));
			memset(buff,0,MSG_LEN);
			
		}

		else if(!(strncmp(buff,"/whois ",7 )))
		{
			msgstruct = whois_func(pollfds[1].fd,msgstruct,buff);
			memset(buff,0,MSG_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
			
		}
		else if(!strncmp(buff,"/msgall ",8))
		{
			msgstruct = msgall_func(pollfds[1].fd,msgstruct,buff);
		}
		else if(!(strncmp(buff,"/msg ",5)))
		{
			memset(&msgstruct,0,sizeof(msgstruct));
			msg_unicast(pollfds[1].fd,msgstruct,buff);
		}
		
		else if(!(strncmp(buff,"/create ",8)))
		{
			//printf("in create\n");

			in_sallon = 1;
			memset(&msgstruct,0,sizeof(msgstruct));
			create_salon(pollfds[1].fd,msgstruct,buff);
		}
		else if((!strncmp(buff,"/channel_liste",14)))
		{
			//printf("in asking channel list\n");
			memset(&msgstruct,0,sizeof(msgstruct));
			ask_channel_list(pollfds[1].fd,msgstruct,buff);
		}
		else if((!strncmp(buff,"/join ",6)))
		{
			in_sallon = 1;
			printf("in join sallone \n");
			memset(&msgstruct,0,sizeof(msgstruct));
			join_sallon(pollfds[1].fd,msgstruct,buff);
		}
		else if((!strncmp(buff,"/quit ",6)))
		{
			in_sallon = 0;
			printf("in quit sallon condition \n");
			memset(&msgstruct,0,sizeof(msgstruct));
			quit_sallon(pollfds[1].fd,msgstruct,buff);
		}
		else{
			//printf("in echo\n");
			//printf("value is  ");
			printf("in sallon is %i \n",in_sallon);
			if(in_sallon == 0)
				msgstruct = send_echo(pollfds[1].fd,msgstruct,buff);
			else{
				msgstruct = send_multicast(pollfds[1].fd,buff);
			}
			if(strcmp(buff,"/quit\n") == 0)
			{
				close(pollfds[1].fd);
				return ;
			}
			memset(buff, 0, MSG_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
			continue;
			
		}
		//cleaning mem
		memset(&msgstruct,0,sizeof(msgstruct));
		memset(buff,0,MSG_LEN);
		}
		else if(pollfds[1].fd & pollfds[1].revents & POLLIN)
		{

		if(recv(pollfds[1].fd,&msgstruct,sizeof(msgstruct),0)<0)
		{
			
			perror("ERROR while recv a strt ");
			break;
		}
		
		if(msgstruct.type == NICKNAME_LIST)
		{
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf("%s", buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == MULTICAST_LIST)
		{
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf("%s", buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == MULTICAST_CREATE)
		{
			printf("Salon already exists\n");
			continue;
		}
		if(msgstruct.type == NICKNAME_INFOS)
		{
			//Receiving message
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf("%s ", buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == BROADCAST_SEND)
		{
			
			memset(buff,0,MSG_LEN);
			char sender[NICK_LEN];
			memset(sender,0,NICK_LEN);
			strncpy(sender,msgstruct.nick_sender,strlen(msgstruct.nick_sender));
			if(recv(pollfds[1].fd,buff,msgstruct.pld_len,0)<0)
			{
				perror("Error while receiving br struct");
				break;
			}
			printf("PUBLIC conv :[%s] sent : %s \n",sender,buff);
		}
		if(msgstruct.type == MULTICAST_QUIT)
		{
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf("%s ", buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == UNICAST_SEND)
		{
			char sender[NICK_LEN];
			memset(sender,0,NICK_LEN);
			strncpy(sender,msgstruct.nick_sender,strlen(msgstruct.nick_sender));
			//receiving message
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf(" PRIVATE conv : [%s] sent %s \n", sender,buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == MULTICAST_SEND)
		{
			char sender[NICK_LEN];
			memset(sender,0,NICK_LEN);
			strncpy(sender,msgstruct.nick_sender,strlen(msgstruct.nick_sender));
			//receiving message
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf(" GROUPE conv : [%s] sent %s \n", sender,buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == ECHO_SEND)
		{
			if (recv(pollfds[1].fd, buff, msgstruct.pld_len, 0) < 0) {
				break;
			}
			printf("%s ", buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		}

		
		
	}
}

int socket_and_connect(char *hostname, char *port) {
	int sock_fd = -1;
	// Création de la socket
	if (-1 == (sock_fd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("Socket");
		exit(EXIT_FAILURE);
	}
	printf("Socket created (%d)\n", sock_fd);
	struct addrinfo hints, *res, *tmp;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	int error;
	error = getaddrinfo(hostname, port, &hints, &res);
	if (error) {
		errx(1, "%s", gai_strerror(error));
		exit(EXIT_FAILURE);
	}
	tmp = res;
	while (tmp != NULL) {
		if (tmp->ai_addr->sa_family == AF_INET) {
			struct sockaddr_in *sockin_ptr = (struct sockaddr_in *)tmp->ai_addr;
			u_short port_number = sockin_ptr->sin_port;
			char *ip_str = inet_ntoa(sockin_ptr->sin_addr);
			printf("Address is %s:%hu\n", ip_str, htons(port_number));
			printf("Connecting...");
			fflush(stdout);
			if (-1 == connect(sock_fd, tmp->ai_addr, tmp->ai_addrlen)) {
				perror("Connect");
				exit(EXIT_FAILURE);
			}
			printf("OK\n");
			freeaddrinfo(res);
			return sock_fd;
		}
		tmp = tmp->ai_next;
	}
	freeaddrinfo(res);
	return -1;
}


int main(int argc, char  *argv[]) {
	if (argc != 3) {
		printf("Usage: ./client hostname port_number\n");
		exit(EXIT_FAILURE);
	}
	char *hostname = argv[1];
	char *port = argv[2];

	int sock_fd = -1;
	if (-1 == (sock_fd = socket_and_connect(hostname, port))) {
		printf("Could not create socket and connect properly\n");
		return 1;
	}

    echo_client(sock_fd);
	close(sock_fd);
	return EXIT_SUCCESS;

	return 0;
}