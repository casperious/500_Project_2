#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/wait.h>
#include "encDec.h"
#include <sys/ioctl.h>
#include <linux/sockios.h>
void errorS(const char *msg)
{
perror(msg);
exit(0);
}
int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	char inp[20480];
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[2048];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		errorS("ERROR opening socket");
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
		errorS("ERROR connecting");
	bzero(buffer,256);
	n = read(sockfd,buffer,255);						//get welcome message on connect
	printf("%s",buffer);
	//get loginList from server, display list of existing usernames and check entered username doesn't clash
	n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);	//send login list request
	if (n < 0)
		errorS("ERROR writing to socket");
	bzero(buffer,256);
	n = read(sockfd,buffer,255);					//get client list from server
	printf("%s",buffer);
	if (n < 0)
		errorS("ERROR reading from socket");
	char* usernames[6] = {"\n","\n","\n","\n","\n","\n"};	//initialize usernames
	for(int i =0;i<6;i++)
	{
		char* name = calloc(9,sizeof(char));
		strncpy(name,buffer+(i*10),8);
		usernames[i] = name;								//set current usernames
		usernames[i][8]='\0';
	}
	printf("\nPlease enter username 8 characters long starting with a letter\n"); 
	char* username = calloc(9,sizeof(char));
	while(1)
	{
		scanf("%s",username);
		username[8]='\0';
		if(strlen(username)!=8)
		{
			printf("Please enter a name 8 characters long, starting with a letter. Entered username is %ld characters long\n",strlen(username));
		}
		else if((username[0]>=65 && username[0]<=90) || (username[0]>=97 && username[0]<=122))
		{
			int idx = -1;
			for(int i =0;i<6;i++)
			{
				if(strcmp(username,usernames[i])==0)				//check to see if username is taken
				{
					idx = i;
					break;
				}
			}
			if(idx==-1)												//if username is available
			{
				char* usernameString = calloc(8+9+9+1,sizeof(char));
				char* loginStart = "<LOGIN>";
				char* loginEnd = "</LOGIN>";
				strcat(usernameString,loginStart);
				strcat(usernameString,username);
				strcat(usernameString,loginEnd);
				usernameString[27]='\0';
				n = write(sockfd,usernameString,27);				//write <LOGIN> message to server
				if(n<0)
				{
					errorS("ERROR writing username to socket\n");
				}
				printf("Welcome %s\n",username);
				break;
			}
			else													//username taken
			{
				printf("Username is taken. Please choose another username\n");
			}
		}
		else														//username doesn't start with a letter
		{
			printf("Please enter a username starting with a letter\n");
		}
	}
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }				//flush stdin
	int mpid;
	int activity;
	fd_set readfds;
	mpid= fork();
	if(mpid==0)													//child process reads input from server
	{	
		char readBuffer[2049]; 
		while(1)
		{
			bzero(readBuffer,2048);
			n = recv(sockfd, readBuffer, 2048, MSG_DONTWAIT);							//Non interrupt socket read
			
			if(strcmp(readBuffer,"Exit\n")==0)
			{	
				printf("Read Exit\n Closing client\n");
				return 0;
				exit(0);
			}
			char* msg = calloc(6,sizeof(char));
            strncpy(msg,readBuffer,5);
            if(strcmp(msg,"<MSG>")==0)
            {
            	char* messageContents = calloc(strlen(readBuffer)-5-6,sizeof(char));
            	strncpy(messageContents,readBuffer+5,strlen(readBuffer)-5-6);			//get contents within <MSG> tag
            	bzero(readBuffer,2048);
            	char* from = calloc(9,sizeof(char));
            	char* to = calloc(9,sizeof(char));
            	char* encode = calloc(2,sizeof(char));
                char* body = calloc(strlen(messageContents)-69,sizeof(char));
                strncpy(from,messageContents+6,8);
                from[8]='\0';															//extract from username	
                strncpy(to,messageContents+25,8);
                to[8]='\0';																//extract to username
                strncpy(encode,messageContents+46,1);
                encode[1]='\0';															//extract encode
                strncpy(body,messageContents+62,strlen(messageContents)-69);			//extract body contents
                int pid1;
                pid1 = fork();
                if(pid1==0)
                {
                	char sock[4];
					sprintf(sock,"%d",sockfd);	
                    if(encode[0]=='h')
                    {
                    	execl("deframe","deframe",body,sock,encode,"\n",NULL);			//call deframe for every frame read
                    }
                    else
                    {
                    	execl("crcCheck","crcCheck",body,sock,encode,"\n",NULL);		//call crcCheck if user inputs c
                    }
                }
                else if(pid1>0)
                {
                    wait(NULL);
                }
                else
                {
                    printf("fork in server to decode body failed\n");
                }
                free(from);
                free(to);
                free(encode);
                free(body);
            }
            free(msg);			
		}		
		
	}
	else if(mpid>0)																				//parent process reads input from user
	{
		while(1)
		{
			printf("Please choose one of the following:\n 1. List of users on the server. \n 2. Send a message to a user. \n 3. Logout\n ");
			bzero(buffer,256);
			int m =0;
			char cha;
			while((cha=getc(stdin))!='\n' && cha>=0 && cha<=127)
			{
				buffer[m]=cha;
				m++;
			}
			if(buffer[0]=='1')															//get client list
			{
				n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);
				if (n < 0)
					errorS("ERROR writing to socket");
				bzero(buffer,256);
				n = read(sockfd,buffer,255);
				if (n < 0)
					errorS("ERROR reading from socket");
				printf("%s",buffer);
			}
			else if(buffer[0]=='2')														//send message
			{
				printf("Please enter the username of the person you would like to send a message to\n");
				bzero(buffer,256);
				cha='\0';
				m=0;
				while((cha=getc(stdin))!='\n')
				{
					buffer[m]=cha;
					m++;
				}
				char* to = calloc(9,sizeof(char));
				strncpy(to,buffer,8);
				to[8]='\0';
				n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);
				if (n < 0)
					errorS("ERROR writing to socket");
				bzero(buffer,2048);
				n = recv(sockfd, buffer, 2048, MSG_DONTWAIT);				//get updated client list
				if (n < 0)
					errorS("ERROR reading from socket");
				char* usernamesCheck[6] = {"\n","\n","\n","\n","\n","\n"};		
				int idxCheck = -1;
				for(int i =0;i<6;i++)
				{
					char* name = calloc(10,sizeof(char));
					strncpy(name,buffer+(i*9),8);
					usernamesCheck[i] = name;
					if(strcmp(usernamesCheck[i],to)==0)						//if receiver username is the same as current client, cannot message	
					{	
						idxCheck = i;
						break;
					}
				}
				if(idxCheck>-1)	
				{
					if(strcmp(to,username)==0)
					{	
						printf("Cannot send message to self. Please enter valid username from the list\n");
					}
					else
					{
						printf("\n ------------------\n Enter h for hamming, or c for crc\n --------------------\n");
						bzero(buffer,2048);
						cha='\0';
						m=0;
						while((cha=getc(stdin))!='\n')					//get h or c for encoder
						{
							buffer[m]=cha;
							m++;
						}
						if(buffer[0]=='h')
						{	
							printf("Enter frame number where you would like error inserted\n");
							bzero(buffer,2048);
							char errC;
							int u=0;
							while((errC=getc(stdin))!='\n')				//get frame number for error insertion
							{
								if(isdigit(errC))
								{
									buffer[u]=errC;
									u++;
								}
								else
								{
									printf("Skipping non digit\n");
								}
							}
							buffer[u]='\0';
							int errFrame = atoi(buffer);
							char errFrameString[64];
							sprintf(errFrameString,"%d",errFrame);
							printf("Please enter the message for hamming\n");
							char ch;
							bzero(inp,20480);
							int l=0;
							while((ch=getc(stdin))!='`')					//get message till ` is detected
							{
								inp[l]=ch;
								l++;
							}
							int pid;
							pid = fork();
							if(pid==0)
							{
								char countStr[64];
								sprintf(countStr,"%d",sockfd);														//store socket to write to into countStr
								execl("producer","producer",countStr,"h",inp,username,to,errFrameString,NULL);		//call producer			
							}
							else if(pid>0)
							{
								wait(NULL);
							}
							else
							{
								printf("Fork in client failed\n");
							}
						}
						else if(buffer[0]=='c')
						{
							printf("Enter frame number where you would like error inserted\n");
							bzero(buffer,2048);
							char errC;
							int u=0;
							while((errC=getc(stdin))!='\n')			//get frame number for error insertion
							{
								if(isdigit(errC))
								{
									buffer[u]=errC;
									u++;
								}
								else
								{
									printf("Skipping non digit\n");
								}
							}
							buffer[u]='\0';
							int errFrame = atoi(buffer);
							char errFrameString[64];
							sprintf(errFrameString,"%d",errFrame);
							printf("Please enter the message for crc\n");
							char ch;
							bzero(inp,20480);
							int l=0;
							while((ch=getc(stdin))!='`')			//get message till ` is detected
							{
								inp[l]=ch;
								l++;
							}
							int pidc;
							pidc = fork();
							if(pidc==0)
							{
								char countStr[64];
								sprintf(countStr,"%d",sockfd);														//store sockfd into countStr
								execl("producer","producer",countStr,"c",inp,username,to,errFrameString,NULL);		//call producer
							}
							else if(pidc>0)
							{
								wait(NULL);
							}
							else
							{
								printf("Fork in client failed\n");
							}
						}
						else
						{
							printf("Wrong option\n");
						}
					}
				}
				else
				{
					printf("\n Username not found. Please enter valid username from the list\n");
				}
				
			}
			else if(buffer[0]=='3')
			{
				n = write(sockfd,"<LOGOUT></LOGOUT>",16);					//write logout to server
				if (n < 0)
					errorS("ERROR writing to socket");
				bzero(buffer,2048);
				n = recv(sockfd, buffer, 2048, MSG_DONTWAIT);				//read confirmation from server		
				printf("%d\n",strcmp(buffer,"Exit\n"));
				if(strcmp(buffer,"Exit\n")==0)
				{	
					printf("Read Exit\n Closing client\n");					//close client
					return 0;
					exit(0);
				}
				if (n < 0)
					errorS("ERROR reading from socket");
				printf("%s",buffer);
			}
			else
			{
				printf("Invalid choice. Please choose 1 2 or 3. Entered %d\n",buffer[0]);
			}	
			
		}
		close(sockfd);
	}
	return 0;
}
