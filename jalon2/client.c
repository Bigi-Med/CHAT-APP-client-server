#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "msg_struct.h"

#define STR_MAX 128

void echo_client(int sockfd) {
	struct message msgstruct;
	char buff[MSG_LEN];
	char user[MSG_LEN];
	int n;
	memset(buff, 0, INFOS_LEN);
	memset(&msgstruct,0,sizeof(struct message));
	
	//receiving first struct

	if(recv(sockfd,&msgstruct,sizeof(msgstruct),0)<=0)
	{
		perror("Receiving ");
		exit(EXIT_FAILURE);
	}

	char msg[msgstruct.pld_len];
	memset(msg,0,msgstruct.pld_len);

	//receiving first message

	if(recv(sockfd,msg,msgstruct.pld_len,0)<=0)
	{
		perror("Receiving ");
		exit(EXIT_FAILURE);
	}
	printf("%s \n",msg);

	
	int k;
	k = 0;
	memset(user, 0, MSG_LEN);

	while ((user[k++] = getchar()) != '\n') {} // trailing '\n' will be sent
	//k = 0;
	printf("buff is %s \n", user);
	//char test[6];
	
	//memset(test,0,6);
	//memcpy(test,user,6);
	fflush(stdout);
	//printf("test is %s \n",test);
	//printf("test lenght %ld \n",strlen(test));
	
	if(strncmp(user,"/nick ",6)==0)
	{
		char name[NICK_LEN];
		memcpy(name,user + 6,sizeof(user)-6);
		printf("name is %s \n",name);
		//setting up the struct
		msgstruct.pld_len = strlen(name)+1;
		strncpy(msgstruct.nick_sender, "\0", 1);
		msgstruct.type = NICKNAME_NEW;
		strncpy(msgstruct.infos, name, strlen(name));
		
		//sending the struct
		if (send(sockfd, &msgstruct, sizeof(msgstruct), 0) < 0) {
			exit(EXIT_FAILURE);
		}
		//sending nick name
		if (send(sockfd, name, msgstruct.pld_len, 0) < 0) {
			exit(EXIT_FAILURE);
		}

		//receiving the welcome message
		

		if(recv(sockfd,&msgstruct,sizeof(msgstruct),0)<0)
		{
			perror("Receiving ");
			exit(EXIT_FAILURE);
		}
		char welcome[msgstruct.pld_len];
		memset(welcome,0,msgstruct.pld_len);

		if(recv(sockfd,welcome,msgstruct.pld_len,0)<0)
		{
			perror("Receiving ");
			exit(EXIT_FAILURE);
		}
		printf("%s \n",welcome);








		
	}
	

	
	/*memset(user, 0, 100);
	printf("pseudo is  : ");*/

	//while ((user[k++] = getchar()) != '\n') {} // trailing '\n' will be sent
	//printf("buff is %s \n", user);
	//char test[6];
	//memset(test,0,6);
	//memcpy(test,user,6);
	//fflush(stdout);
	//printf("test is %s \n",test);
	//printf("frst %c \n",user[0]);
	//printf("scnd %c \n",user[1]);
	/*if(user[0] == '/' && user[1] == 'n' && user[2] == 'i' && user[3] == 'c' && user[4] == 'k' && user[5] == ' ')
		{	
			
			
			
		}*/
		
	//else{
	while (1) {
		// Cleaning memory
		memset(buff, 0, INFOS_LEN);
		memset(&msgstruct,0,sizeof(struct message));
		// Getting message from client
		printf("Message: ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
		/*if(strcmp(buff,"/nick\n") == 0)
		{	
			
			echo_nick(sockfd);
			
		}*/

		//else{
		//filling structur
		msgstruct.pld_len = strlen(buff);
		strncpy(msgstruct.nick_sender, "simo", 4);
		msgstruct.type = ECHO_SEND;	
		strncpy(msgstruct.infos, "\0", 1);
		// Sending structure
		if (send(sockfd, &msgstruct, sizeof(msgstruct), 0) <= 0) {
			break;
		}
		// Sending message (ECHO)
		if (send(sockfd, buff, msgstruct.pld_len, 0) <= 0) {
			break;
		}
		printf("Message sent!\n");
		// Cleaning memory
		memset(buff, 0, INFOS_LEN);
		// reveiving struct
		printf("before rec \n");
		if(recv(sockfd,&msgstruct,sizeof(msgstruct),0)<=0)
			break;
		// Receiving message
		printf("before rec 2\n");
		/*if (recv(sockfd, buff, MSG_LEN, 0) <= 0) {
			break;
		}
		printf("Received: %s", buff);*/
		}
	//}
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
			return sock_fd;
		}
		tmp = tmp->ai_next;
	}
	return -1;
}

int main(int argc, char  *argv[]) {
	
	 int track = 0;

	printf("track before : %d",track);
	
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

    
	//echo_nick(sock_fd);
		

	


	echo_client(sock_fd);
	close(sock_fd);
	return EXIT_SUCCESS;

	return 0;
}