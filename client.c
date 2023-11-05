#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
void error(const char *msg)
{
perror(msg);
exit(0);
}
int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[1024];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	bzero(buffer,256);
	n = read(sockfd,buffer,255);
	printf("%s\n",buffer);
	//get loginList from server, display list of existing usernames and check entered username doesn't clash
	printf("Please enter username 8 characters long starting with a letter\n"); 
	char* username = calloc(9,sizeof(char));
	while(1)
	{
		scanf("%s",username);
		if(strlen(username)!=8)
		{
			printf("Please enter a name 8 characters long, starting with a letter. Entered username is %ld characters long\n",strlen(username));
		}
		else if((username[0]>=65 && username[0]<=90) || (username[0]>=97 && username[0]<=122))
		{
			char* usernameString = calloc(8+9+9+1,sizeof(char));
			char* loginStart = "<LOGIN>";
			char* loginEnd = "</LOGIN>";
			strcat(usernameString,loginStart);
			strcat(usernameString,username);
			strcat(usernameString,loginEnd);
			usernameString[27]='\0';
			n = write(sockfd,usernameString,27);
			if(n<0)
			{
				error("ERROR writing username to socket\n");
			}
			printf("Welcome %s\n",username);
			break;
		}
		else
		{
			printf("Please enter a username starting with a letter\n");
		}
	}
	int c;
	free(username);
	while ((c = getchar()) != '\n' && c != EOF) { }				//flush stdin
	while(1)
	{
		printf("Please choose one of the following:\n 1. List of users on the server. \n 2. Send a message to a user. \n 3. Logout\n ");
		bzero(buffer,256);
		fgets(buffer,255,stdin);
		if(buffer[0]=='1')
		{
			n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);
			if (n < 0)
				error("ERROR writing to socket");
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0)
				error("ERROR reading from socket");
			printf("%s",buffer);
		}
		else if(buffer[0]=='2')
		{
			printf("Please enter the username of the person you would like to send a message to\n");
			bzero(buffer,256);
			fgets(buffer,255,stdin);
			n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);
			if (n < 0)
				error("ERROR writing to socket");
			bzero(buffer,256);
			n = read(sockfd,buffer,1024);
			if (n < 0)
				error("ERROR reading from socket");
			char* usernames[6] = {"\n","\n","\n","\n","\n","\n"};
			
			for(int i =0;i<6;i++)
			{
				char* name = calloc(10,sizeof(char));
				strncpy(name,buffer+(i*9),8);
				usernames[i] = name;
				if(usernames[i][0]=='\n')
				{
					break;
				}
				//printf("User %d is %s\n",i,usernames[i]);
				//free(usernames[i]);	//comment out once checked
			}
			
			
		}
		else if(buffer[0]=='3')
		{
			n = write(sockfd,"<LOGOUT></LOGOUT>",16);
			if (n < 0)
				error("ERROR writing to socket");
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			printf("%d\n",strcmp(buffer,"Exit\n"));
			if(strcmp(buffer,"Exit\n")==0)
			{
				printf("Read Exit\n Closing client\n");
				break;
			}
			if (n < 0)
				error("ERROR reading from socket");
			printf("%s",buffer);
		}
		else
		{
			printf("Invalid choice. Please choose 1 2 or 3\n");
		}
		//n = write(sockfd,buffer,strlen(buffer));
		/*
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		printf("%d\n",strcmp(buffer,"Exit\n"));
		if(strcmp(buffer,"Exit\n")==0)
		{
			printf("Read Exit\n Closing client\n");
			//while ((c = getchar()) != '\n' && c != EOF) { }	
			//close(sockfd);
			break;
		}
		if (n < 0)
			error("ERROR reading from socket");
		printf("%s",buffer);*/
	}
	//n = write(sockfd,"Done",4);
	close(sockfd);
	return 0;
}
