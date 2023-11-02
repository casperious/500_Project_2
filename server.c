#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
int main(int argc, char *argv[])
{
	int limit = 6;
	int count = 0;
	int opt=1;
	FILE* clientList;
	clientList = fopen("clientList.txt","w");
	int sockfd, portno;
	int clientSockets[6];
	int maxClients=6;
	fd_set readfds;
	socklen_t clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
	fprintf(stderr,"ERROR, no port provided\n");
	exit(1);
	}
	fprintf(stdout, "Run client by providing host and port\n");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	if (sockfd < 0)
		error("ERROR opening socket");
	 if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	//char exit[10];
	//bzero(exit,10);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	 //try to specify maximum of 3 pending connections for the master socket  
    if (listen(sockfd, 5) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
    clilen = sizeof(cli_addr);
    int max_sd;
    int sd,i,activity,new_socket,valread;
    char* message = "Welcome to the server!\n";
	while(1)
	{	
		//printf("listening at start of inf loop %d\n",count);	
		//listen(sockfd,5);
		 FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(sockfd, &readfds);   
        max_sd = sockfd;  
        //add child sockets to set  
        for ( i = 0 ; i < maxClients ; i++)   
        {   
            //socket descriptor  
            sd = clientSockets[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }    
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
        
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(sockfd, &readfds))   
        {   
            if ((new_socket = accept(sockfd,  
                    (struct sockaddr *)&serv_addr, (socklen_t*)&clilen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port));   
           
            //send new connection greeting message  
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }   
                 
            puts("Welcome message sent successfully");   
                 
            //add new socket to array of sockets  
            for (i = 0; i < maxClients; i++)   
            {   
                //if position is empty  
                if( clientSockets[i] == 0 )   
                {   
                    clientSockets[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }
                
        }
         for (i = 0; i < maxClients; i++)   
        	{   
            	sd = clientSockets[i];   
                 
            	if (FD_ISSET( sd , &readfds))   
            	{   
               	 //Check if it was for closing , and also read the  
               	 //incoming message  
               	 	valread = read( sd , buffer, 1024);
               	 	//printf("%d is valread for %s\n",valread,buffer);
               		if (valread == 0)   
           		    {   
                    	//Somebody disconnected , get his details and print  
                    	getpeername(sd , (struct sockaddr*)&serv_addr , (socklen_t*)&clilen);   
                    	printf("Host disconnected , ip %s , port %d \n" ,  
                    	      inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port));   
                         
                    	//Close the socket and mark as 0 in list for reuse  
                    	close( sd );   
                    	clientSockets[i] = 0;   
                	}   
                     
                	//Echo back the message that came in  
                	else 
                	{   
                    	//set the string terminating NULL byte on the end  
                    	//of the data read  
                    	buffer[valread] = '\0';   
                    	send(sd , buffer , strlen(buffer) , 0 );   
                	}   
            	}   
        	}   
		/*if(count<6)
		{
			
			newsockfdList[count] = accept(sockfd,
			(struct sockaddr *) &cli_addr,&clilen);
			//int newsock = newsockfdList[count];
		//fgets(exit,10,stdin);
		//printf("%s is the command entered\n",exit);
		//if(strcmp(exit,"Exit\n")==0)
		//{
		//	break;
		//}
			
			if (newsockfdList[count] < 0)
				error("ERROR on accept");
			else
			{
				printf("Added client at %d index %d\n",newsockfdList[count],count);
				count++;
			}
		}
		else
		{
			printf("Hit client limit\n");
		}
		
		for(int i =0;i<=count;i++)
		{
			printf("in for loop\n");
			bzero(exit,10);
			bzero(buffer,256);
			n = read(newsockfdList[i],buffer,255);
			printf("%d",strcmp(buffer,"Exit\n"));
			if(strcmp(buffer,"Exit\n")==0)
			{
				//printf("Read Exit closing server\n");
				n = write(newsockfdList[i],"Exit\n",5);
				bzero(buffer,256);
				n = read(newsockfdList[i],buffer,4);
				if(strcmp(buffer,"Done")==0)
				{
					printf("Closing client socket\n");
					close(newsockfdList[i]);
					for(int j = i;j<count;j++)
					{
						newsockfdList[j]=newsockfdList[j+1];
					}
					count--;
					break;
				}
				else
				{
					printf("Waiting for client to end\n");
				}
					
			}
			else
			{
				if (n < 0)
					error("ERROR reading from socket");
				printf("Here is the message: %s\n",buffer);
				n = write(newsockfdList[i],"I got your message",18);
				if (n < 0)
					error("ERROR writing to socket");
			}
		}*/
	}
	//close(newsockfd);
	close(sockfd);
	fclose(clientList);
	return 0;
}
