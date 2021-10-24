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
#define BACKLOG 1
#define MIN(x,y) ((x)<(y)?(x):(y))

char nick[NICK_LEN];
char salon[NICK_LEN];
char ip_str[MSG_LEN];
char port[MSG_LEN];
char myfile[NICK_LEN];
char file_rec[]="File was received \n";
char file_nrec[]="File not received ! \n";
int is_there_conn = 0;



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
	while ((buff[k++] = getchar()) != '\n') {}
	
	char new_nick[NICK_LEN];
	memset(new_nick,0,NICK_LEN);
	memcpy(new_nick,buff+6,strlen(buff)-7);
	

	int L = strlen(new_nick);
	int i = 0;
	while(i<L)
	{
		if((new_nick[i]<'A') || (new_nick[i]>'Z' && new_nick[i]<'a') || (new_nick[i]>'z') || new_nick[i] == ' ' )
		{
		
		printf("Pseudo must not contain special characters, numbers or spaces, enter pseudo again : \n");
		memset(buff,0,MSG_LEN);
		memset(new_nick,0,NICK_LEN);
		k = 0;
		while ((buff[k++] = getchar()) != '\n') {}
		memcpy(new_nick,buff+6,strlen(buff)-7);
		continue;
		}
		else{
			i++;
		}
	}

	
	
	while(strncmp(buff,"/nick ",6)!=0)
	{
		printf("Please enter a pseudo \n");
		memset(buff,0,MSG_LEN);
		k = 0;
		while ((buff[k++] = getchar()) != '\n') {}
		
	
	}
	
	//retreiving the nick name

	//char nick[NICK_LEN];
	memset(nick,0,NICK_LEN);
	memcpy(nick,buff +6,strlen(buff)-7);
	memset(buff,0,MSG_LEN);
	
	memset(&msg,0,sizeof(msg));
	strncpy(msg.nick_sender,"\0",1);
	msg.type = NICKNAME_NEW;
	strncpy(msg.infos,nick,strlen(nick));
	msg.pld_len = strlen(msg.infos);

	//sending the struct containing the nick name
	
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
		
	// Sending message (ECHO)
	if (send(sockfd, buff, msg.pld_len, 0) < 0) {
		perror("Error while sending message");
		//break;
	}

	printf("Message sent!\n");
	// Cleaning memory
	
		return msg;
}

void file_request(int sockfd)
{
	struct message msgstruct;
	int n;
	memset(&msgstruct,0,sizeof(msgstruct));
	msgstruct.type = FILE_REQUEST;
	strncpy(msgstruct.nick_sender , nick,strlen(nick));
	char destination[NICK_LEN];
	char file_name[NICK_LEN];
	memset(destination,0,NICK_LEN);
	memset(file_name,0,NICK_LEN);
	n = 0;
	printf("Enter client to send file to : \n");
	while ((destination[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
	n = 0;
	printf("Enter file name to send : \n");
	while ((file_name[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
	strncpy(msgstruct.infos,destination,strlen(destination)-1);
	memset(myfile,0,NICK_LEN);
	strncpy(myfile,file_name,strlen(file_name)-1);
	msgstruct.pld_len = strlen(file_name)-1;
	//sending the struct containing the destination
	if(send(sockfd,&msgstruct,sizeof(msgstruct),0)<0)
	{
		perror("Error while sending file request struct");
	}
	//sendig payload containing file name
	if(send(sockfd,file_name,msgstruct.pld_len,0)<0)
	{
		perror("Error while sending file request payload");
	}
	
	memset(&msgstruct,0,sizeof(msgstruct));
	memset(destination,0,NICK_LEN);
	//memset(file_name,0,NICK_LEN);
	return;

}

void file_ack(int sockfd,long to_rec, int rec ,char *file)
{
	struct message msg;
	memset(&msg,0,sizeof(msg));
	
	if(to_rec == rec)
	{
		msg.pld_len = strlen(file_rec);
		msg.type = FILE_ACK;
		strncpy(msg.infos,file,strlen(file));
		strncpy(msg.nick_sender,nick,strlen(nick));
		if(send(sockfd,&msg,sizeof(msg),0)<0)
		{
			perror("Error while sneding ack struct");
		}
		if(send(sockfd,file_rec,msg.pld_len,0)<0)
		{
			perror("Error while sending file ack msg");
		}
	}
	else{
		msg.pld_len = strlen(file_nrec);
		msg.type = FILE_ACK;
		strncpy(msg.infos,file,strlen(file));
		strncpy(msg.nick_sender,nick,strlen(nick));
		if(send(sockfd,&msg,sizeof(msg),0)<0)
		{
			perror("Error while sneding ack struct");
		}
		if(send(sockfd,file_rec,msg.pld_len,0)<0)
		{
			perror("Error while sending file ack msg");
		}


	}
	
}

void gestion_request(int sockfd,struct message msg)
{
	printf("[%s] wants you to accept the transfer of the file named [%s] , DO YOU ACCEPT [Y/N] \n",msg.nick_sender,msg.infos);
	int n;
	n = 0;
	char buff[MSG_LEN];
	char payload[MSG_LEN];
	memset(payload,0,MSG_LEN);
	memset(buff,0,MSG_LEN);
	char source[NICK_LEN];
	memset(source,0,NICK_LEN);
	strncpy(source,msg.nick_sender,strlen(msg.nick_sender));
	strcpy(payload,ip_str);
	strcat(payload,":");
	strcat(payload,port);
	while ((buff[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
	if(strncmp(buff,"Y",1)==0)
	{
		memset(&msg,0,sizeof(msg));
		msg.type = FILE_ACCEPT;
		strncpy(msg.nick_sender,nick,strlen(nick));
		strncpy(msg.infos,source,strlen(source));
		msg.pld_len = strlen(payload);
		//sendig struct
		if(send(sockfd,&msg,sizeof(msg),0)<0)
		{
			perror("Error while sending accept struct ");
		}
		if(send(sockfd,payload,msg.pld_len,0)<0)
		{
			perror("Error while sending acc message");
		}

		//******************************//
		struct sockaddr_in saddr_in;
		int sock = -1;
		static int new_port ;
		new_port = atoi(port)+1;
		sock = socket(AF_INET,SOCK_STREAM,0);
		if(sock == -1)
		{
			perror("SOCKET 3");
			exit(EXIT_FAILURE);
		}
		memset(&saddr_in,0,sizeof(saddr_in));
		saddr_in.sin_family = AF_INET;
		saddr_in.sin_port = htons(new_port);
		saddr_in.sin_addr.s_addr = INADDR_ANY;
		if(bind(sock,(struct sockaddr *)&saddr_in,sizeof(saddr_in))==-1)
		{
			perror("Bind");
			exit(EXIT_FAILURE);
		}
		if(listen(sock,BACKLOG)==-1)
		{
			perror("listen 2");
			exit(EXIT_FAILURE);
		}
		printf("listening on socket %i and port %hd \n",sock,new_port);
		struct sockaddr client_addr;
		socklen_t size = sizeof(client_addr);
		int client_fd;
		if (-1 == (client_fd = accept(sock, &client_addr, &size))) {
			perror("Accept");
			exit(EXIT_FAILURE);
		}
		
		FILE *F = fopen("recv.txt","a");
		struct message file_recv;
		memset(&file_recv,0,sizeof(file_recv));
		/*char testt[MSG_LEN];
		memset(testt,0,MSG_LEN);*/
		char file_to[MSG_LEN];
		memset(file_to,0,MSG_LEN);
		if(recv(client_fd,&file_recv,sizeof(file_recv),0)<0)
		{
			perror("Error while recv file send struct");
		}
		if(recv(client_fd,file_to,file_recv.pld_len,0)<0)
		{
			perror("Error while receiving file send");
		}
		fseek(F, 0, SEEK_END);
		long Fsize = ftell(F);
		fputs(file_to,F);
		file_ack(client_fd,Fsize,file_recv.pld_len,myfile);
		/*if(recv(client_fd,testt,6,0)<0)
		{
			perror("Error re");
		}*/
		//printf("received file %s\n",testt);
		
		new_port++;
		close(client_fd);
		close(sock);
		fclose(F);

	}
	else{
		memset(&msg,0,sizeof(msg));
		msg.type = FILE_REJECT;
		strncpy(msg.nick_sender,nick,strlen(nick));
		strncpy(msg.infos,source,strlen(source));
		msg.pld_len = 0;
		//sendig struct
		if(send(sockfd,&msg,sizeof(msg),0)<0)
		{
			perror("Error while sending accept struct ");
		}
		
	}

}

void get_ad(char *buff,char *to_change)
{
	
	memset(to_change,0,MSG_LEN);
	int length = (int)strlen(buff);
	int i = 1;
	
	while(i<length)
	{
		if(buff[i]!=':')
		{
			i++;
			continue;
		}
		else{
			break;
		}
	}
	
	strncpy(to_change,buff,i);
	
	

}

int send_file(char *buff,struct message msg)
{
	char ip_adr[MSG_LEN];
	get_ad(buff,ip_adr);
	printf("%s \n",ip_adr);
	static int new_port ;
	new_port= atoi(port)+1;
	
	int sockfd = -1;
	struct sockaddr_in addr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);	
	if(sockfd == -1)
	{
		perror("Socket 2");
		exit(EXIT_FAILURE);
	}
	printf("Socket 2 created (%d)\n",sockfd);
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(new_port);
	inet_aton(ip_adr,&addr.sin_addr);
	if(-1 == connect(sockfd,(struct sockaddr *)&addr,sizeof(addr)))
	{
		perror("Connect 2");
		exit(EXIT_FAILURE);
	}
	
	FILE *f = fopen(myfile, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

	char *string = malloc(fsize + 1);
	fread(string, 1, fsize, f);
	fclose(f);

	string[fsize] = 0;

	printf("P2P SENDER SOCKET CREATED \n");
	/*printf("testin sending a char \n");
	char test[] = "yarbi";*/
	struct message file_msg;
	memset(&file_msg,0,sizeof(file_msg));
	file_msg.pld_len = fsize;
	strncpy(file_msg.infos,myfile,strlen(myfile));
	strncpy(file_msg.nick_sender,nick,strlen(nick));
	file_msg.type = FILE_SEND;
	//SENDING STRUCT
	if(send(sockfd,&file_msg,sizeof(file_msg),0)<0)
	{
		perror("Error while sending file send struct");
	}

	if(send(sockfd,string,fsize,0)<0)
	{
		perror("Error while sending file send file");
	}
	/*if(send(sockfd,test,6,0)<0)
	{
		perror("Er");
	}*/
	
	struct message msgg;
	memset(&msgg,0,sizeof(msgg));
	char ack[MSG_LEN];
	memset(ack,0,MSG_LEN);
	if(recv(sockfd,&msgg,sizeof(msgg),0)<0)
	{
		perror("Error while recv ack struct");
	}
	if(recv(sockfd,ack,msgg.pld_len,0)<0)
	{
		perror("Error while recv ack msg");
	}
	printf("%s \n",ack);
	new_port++;
	close(sockfd);

	return sockfd;

}




void echo_client(int sockfd) {
	static int in_sallon = 0;
	
	//int checkk = 0;
	char buff[MSG_LEN];
	
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
		
		ask_for_nick(pollfds[1].fd,msgstruct);
		memset(&msgstruct,0,sizeof(msgstruct));
		memset(buff,0,MSG_LEN);
		//receiving nick response struct

		if(recv(pollfds[1].fd,&msgstruct,sizeof(msgstruct),0)<0)
		{
			perror("Error while receiving nick response struct");
		}
		
		if(!strncmp(msgstruct.nick_sender,"\0",1))
		{
			
			printf("Pseudo already taken, enter a new one \n");
			continue;
		}
		else{
			//receive welcome message and leave loop
			
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
			
			memset(&msgstruct,0,sizeof(msgstruct));
			join_sallon(pollfds[1].fd,msgstruct,buff);
		}
		else if((!strncmp(buff,"/quit ",6)))
		{
			in_sallon = 0;
			
			memset(&msgstruct,0,sizeof(msgstruct));
			quit_sallon(pollfds[1].fd,msgstruct,buff);
		}
		else if((!strncmp(buff,"/send",5)))
		{
			
			memset(buff,0,MSG_LEN);
			file_request(pollfds[1].fd);
			

		}
		else{
			//printf("in echo\n");
			//printf("value is  ");
			
			if(in_sallon == 0)
				msgstruct = send_echo(pollfds[1].fd,msgstruct,buff);
			else{
				msgstruct = send_multicast(pollfds[1].fd,buff);
			}

			if(strcmp(buff,"/quit\n") == 0)
			{
				if(in_sallon == 1)
				{	printf("you need to leave salon first \n");
					continue;
				}
				else{
				close(pollfds[1].fd);
				return ;
				}
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
			if(strncmp(msgstruct.infos,salon,MIN(strlen(msgstruct.infos),strlen(salon)))!=0 && strlen(salon)!=0)
			{
				printf("in cond\n");
				memset(buff,0,MSG_LEN);
				continue;
			}
			printf("%s ", buff);
			memset(buff,0,NICK_LEN);
			memset(&msgstruct,0,sizeof(msgstruct));
		}
		if(msgstruct.type == UNICAST_SEND)
		{
			if(strcmp(msgstruct.nick_sender,nick)==0)
			{
				printf("User [%s] does not exist \n",msgstruct.infos);
				continue;
			}
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
		if(msgstruct.type == FILE_REQUEST)
		{
			gestion_request(pollfds[1].fd,msgstruct);
		}
		if(msgstruct.type == FILE_ACCEPT)
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
			printf("[%s] has accepted the connection and hes is on add:port  : %s \n",sender,buff);
			send_file(buff, msgstruct);
			is_there_conn = 1;
		}
		if(msgstruct.type == FILE_REJECT)
		{
			memset(buff,0,MSG_LEN);
			char sender[NICK_LEN];
			memset(sender,0,NICK_LEN);
			strncpy(sender,msgstruct.nick_sender,strlen(msgstruct.nick_sender));
			
			printf("[%s] has Rrejected the connection \n",sender);
			is_there_conn = 0;
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
	memset(ip_str,0,MSG_LEN);
	while (tmp != NULL) {
		if (tmp->ai_addr->sa_family == AF_INET) {
			struct sockaddr_in *sockin_ptr = (struct sockaddr_in *)tmp->ai_addr;
			u_short port_number = sockin_ptr->sin_port;
			strcpy(ip_str,inet_ntoa(sockin_ptr->sin_addr));
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
	memset(port,0,MSG_LEN);
	char *hostname = argv[1];
	strcpy(port,argv[2]);

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