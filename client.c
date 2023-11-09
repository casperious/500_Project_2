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
	n = read(sockfd,buffer,255);
	printf("%s",buffer);
	//get loginList from server, display list of existing usernames and check entered username doesn't clash
	n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);
	if (n < 0)
		errorS("ERROR writing to socket");
	bzero(buffer,256);
	//printf("Reading client list from sockfd %d\n",sockfd);
	n = read(sockfd,buffer,255);
	if (n < 0)
		errorS("ERROR reading from socket");
	//printf("client list is %s",buffer);
	char* usernames[6] = {"\n","\n","\n","\n","\n","\n"};
	//int idx = -1;
	for(int i =0;i<6;i++)
	{
		char* name = calloc(10,sizeof(char));
		strncpy(name,buffer+(i*9),8);
		usernames[i] = name;
	/*	if(strcmp(username,name)==0)
		{
			idx = i;
			break;
		}
	*/	if(name[0]=='\n')
		{
			break;
		}
		printf("User %d is %s\n",i,usernames[i]);
		//free(usernames[i]);	//comment out once checked
	}
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
			int idx = -1;
			for(int i =0;i<6;i++)
			{
				if(strcmp(username,usernames[i])==0)
				{
					idx = i;
					break;
				}
				if(usernames[i][0]=='\n')
				{
					break;
				}
				//printf("User %d is %s\n",i,usernames[i]);
				//free(usernames[i]);	//comment out once checked
			}
			if(idx==-1)
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
					errorS("ERROR writing username to socket\n");
				}
				printf("Welcome %s\n",username);
				break;
			}
			else
			{
				printf("Username is taken. Please choose another username\n");
			}
		}
		else
		{
			printf("Please enter a username starting with a letter\n");
		}
	}
	int c;
	//free(username);
	while ((c = getchar()) != '\n' && c != EOF) { }				//flush stdin
	int mpid;
	int activity;
	fd_set readfds;
	mpid= fork();
	if(mpid==0)
	{	
		//printf("In client reader\n");
		char readBuffer[2049]; 
		while(1)
		{
			bzero(readBuffer,2048);
			/*int pending;
			ioctl(sockfd, SIOCINQ, &pending);
			activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
			*/
			//n = read(sockfd,readBuffer,2048);				//have to stop freezing
			n = recv(sockfd, readBuffer, 2048, MSG_DONTWAIT);
			
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
            	//printf("In MSG server %s \n", buffer);
            	char* messageContents = calloc(strlen(readBuffer)-5-6,sizeof(char));
            	strncpy(messageContents,readBuffer+5,strlen(readBuffer)-5-6);
            	bzero(readBuffer,2048);
            	//printf("Message contents are %s\n",messageContents);
            	char* from = calloc(9,sizeof(char));
            	char* to = calloc(9,sizeof(char));
            	char* encode = calloc(2,sizeof(char));
                char* body = calloc(strlen(messageContents)-69,sizeof(char));
                strncpy(from,messageContents+6,8);
                from[8]='\0';
               // printf("Message from %s\n",from);
                strncpy(to,messageContents+25,8);
                to[8]='\0';
                //printf("To is %s\n", to);
                strncpy(encode,messageContents+46,1);
                encode[1]='\0';
                //printf("Encode is %s\n",encode);
                strncpy(body,messageContents+62,strlen(messageContents)-69);
                //printf("Body contents are %s\n",body);
                int pid1;
                pid1 = fork();
                if(pid1==0)
                {
                	char sock[4];
					sprintf(sock,"%d",sockfd);	
                    if(encode[0]=='h')
                    {
                    	//printf("Calling deframe on %s\n",body);
                    	execl("deframe","deframe",body,sock,encode,"\n",NULL);			//call deframe for every frame read
                    }
                    else
                    {
                    	execl("crcCheck","crcCheck",body,sock,encode,"\n",NULL);
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
	else if(mpid>0)
	{
		while(1)
		{
			//fflush(stdin);
			printf("Please choose one of the following:\n 1. List of users on the server. \n 2. Send a message to a user. \n 3. Logout\n ");
			bzero(buffer,256);
			int m =0;
			char cha;
			while((cha=getc(stdin))!='\n' && cha>=0 && cha<=127)
			{
				buffer[m]=cha;
				//printf("%c",inp[m]);
				m++;
			}
			//fgets(buffer,255,stdin);
			//while ((c = getchar()) != '\n' && c != EOF) { }				//flush stdin
			if(buffer[0]=='1')
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
			else if(buffer[0]=='2')	
			{
				printf("Please enter the username of the person you would like to send a message to\n");
				bzero(buffer,256);
				cha='\0';
				m=0;
				while((cha=getc(stdin))!='\n')
				{
					buffer[m]=cha;
					//printf("%c",inp[l]);
					m++;
				}
				//fgets(buffer,255,stdin);
				char* to = calloc(9,sizeof(char));
				strncpy(to,buffer,8);
				to[8]='\0';
				n = write(sockfd,"<LOGIN_LIST></LOGIN_LIST>",26);
				if (n < 0)
					errorS("ERROR writing to socket");
				bzero(buffer,2048);
				//n = read(sockfd,buffer,1024);
				n = recv(sockfd, buffer, 2048, MSG_DONTWAIT);
				//printf("Got client list\n");
				if (n < 0)
					errorS("ERROR reading from socket");
				char* usernames[6] = {"\n","\n","\n","\n","\n","\n"};
				int idx = -1;
				for(int i =0;i<6;i++)
				{
					char* name = calloc(10,sizeof(char));
					strncpy(name,buffer+(i*9),8);
					usernames[i] = name;
					if(strcmp(usernames[i],to)==0)
					{
						idx = i;
						break;
					}
					if(usernames[i][0]=='\n')
					{
						
						break;
					}
					//printf("User %d is %s\n",i,usernames[i]);
					//free(usernames[i]);	//comment out once checked
				}
				if(idx>-1)	
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
						while((cha=getc(stdin))!='\n')
						{
							buffer[m]=cha;
							//printf("%c",inp[l]);
							m++;
						}
						//fgets(buffer,2048,stdin);
						if(buffer[0]=='h')
						{	
							printf("Enter frame number where you would like error inserted\n");
							bzero(buffer,2048);
							char errC;
							int u=0;
							while((errC=getc(stdin))!='\n')
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
							//bzero(buffer,2048);
							//scanf("%s",buffer);
							char ch;
							bzero(inp,20480);
							int l=0;
							while((ch=getc(stdin))!='`')
							{
								inp[l]=ch;
								//printf("%c",inp[l]);
								l++;
							}
							//while ((c = getchar()) != '\n' && c != EOF) { }				//flush stdin
							//fgets(buffer,2048,stdin);
							int pid;
							pid = fork();
							if(pid==0)
							{
								//printf("In child calling producer\n");
								char countStr[64];
								sprintf(countStr,"%d",sockfd);														//store length of last block into countStr
								//printf("username in client is %s and to is %s\n",username,to);
								execl("producer","producer",countStr,"h",inp,username,to,errFrameString,NULL);					//was buffer
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
							while((errC=getc(stdin))!='\n')
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
							while((ch=getc(stdin))!='`')
							{
								inp[l]=ch;
								//printf("%c",inp[l]);
								l++;
							}
							//while ((c = getchar()) != '\n' && c != EOF) { }				//flush stdin
							//fgets(buffer,2048,stdin);
							int pidc;
							pidc = fork();
							if(pidc==0)
							{
								//printf("In child calling producer\n");
								char countStr[64];
								sprintf(countStr,"%d",sockfd);														//store length of last block into countStr
								//printf("username in client is %s and to is %s\n",username,to);
								execl("producer","producer",countStr,"c",inp,username,to,errFrameString,NULL);
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
				n = write(sockfd,"<LOGOUT></LOGOUT>",16);
				if (n < 0)
					errorS("ERROR writing to socket");
				bzero(buffer,2048);
				//n = read(sockfd,buffer,255);
				n = recv(sockfd, buffer, 2048, MSG_DONTWAIT);
				printf("%d\n",strcmp(buffer,"Exit\n"));
				if(strcmp(buffer,"Exit\n")==0)
				{	
					printf("Read Exit\n Closing client\n");
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
	}
	return 0;
}
