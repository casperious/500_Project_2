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
#include "encDec.h"
/*struct usernameMap {
	int i;
	char* username;
}
*/

void errorS(const char *msg)
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
	for(int i =0;i<6;i++)
	{
		fputs("\n",clientList);
	}
	int sockfd, portno;
	int clientSockets[6] = {-1,-1,-1,-1,-1,-1};
	//struct usernameMap username[6];
	char* usernames[6] = {"\n","\n","\n","\n","\n","\n"};
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
		errorS("ERROR opening socket");
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
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		errorS("ERROR on binding");
    if (listen(sockfd, 1) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
    clilen = sizeof(cli_addr);
    int max_sd;
    int sd,i,activity,new_socket,valread;
    //int count = 0;
    count=0;
    char* message = "<info>Welcome to the server!</info>\n";
	while(1){
		
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
      	if (FD_ISSET(sockfd, &readfds) && count<6)   
       	{   
        	if ((new_socket = accept(sockfd, (struct sockaddr *)&serv_addr, (socklen_t*)&clilen))<0)   
        	{   
        	    perror("accept");   
        	    exit(EXIT_FAILURE);   
        	}   
        	count++; 
        	//inform user of socket number - used in send and receive commands  
        	printf("New connection , socket fd is %d , ip is : %s , port : %d  count is now %d\n" , new_socket , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port),count);
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
        	    if( clientSockets[i] == -1 )   
        	    {   
        	        clientSockets[i] = new_socket;   
        	        printf("Adding to list of sockets at %d\n" , i);   
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
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port));   
                    //Close the socket and mark as -1in list for reuse  
                    close( sd );   
                    if(usernames[i][0]!='\n')
                    {
                    	free(usernames[i]);
                    }
                    usernames[i]="\n";
                    FILE* tmp = fopen("tmp.txt","w");
                    for(int j =0;j<6;j++)
                    {
                    	fputs(usernames[j],tmp);
                    }
                    fclose(clientList);
                    fclose(tmp);
                    remove("clientList.txt");
        	   		rename("tmp.txt","clientList.txt");
        	   		clientList = fopen("clientList.txt","r");
                    clientSockets[i] = -1;   
                    count--;
                }   
                //Echo back the message that came in  
                else 
                {   
                	if(strcmp(usernames[i],"\n")==0)
               	 	{
               	 		char* usernameString = buffer;
        	  			printf("username string read by server is %s\n",usernameString);
        	   			char* username = calloc(10,sizeof(char));
        	   			strncpy(username,usernameString+7,8);
        	   			
        	   			username[8]='\n';
        	   			username[9]='\0';
        	   			printf("username is %s at port %d\n",username,sd);
        	   			usernames[i] = username;
        	   			FILE* tmp = fopen("tmp.txt","w");
        	   			for(int j=0;j<6;j++)
        	   			{
        	   				//printf("Writing %s to tmp\n",usernames[j]);
        	   				fputs(usernames[j],tmp);
        	   			}
        	   			fclose(clientList);
        	   			fclose(tmp);
        	   			remove("clientList.txt");
        	   			rename("tmp.txt","clientList.txt");
        	   			clientList = fopen("clientList.txt","r");
        	   			
               	 	}
               	 	else
               	 	{
                    	//set the string terminating NULL byte on the end  
                    	//of the data read  
                    	buffer[valread] = '\0';   
                    	char* msg = calloc(6,sizeof(char));
                    	char* loginList = calloc(13,sizeof(char));
                    	char* logout = calloc(9,sizeof(char));
                    	strncpy(logout,buffer,8);
                    	strncpy(loginList,buffer,12);
                    	strncpy(msg,buffer,5);
                    	if(strcmp(msg,"<MSG>")==0)
                    	{
                    		char* messageContents = calloc(strlen(buffer)-5-6-1,sizeof(char));
                    		strncpy(messageContents,buffer+5,strlen(buffer)-5-6-1);
                    		printf("Message contents are %s\n",messageContents);
                    	}
                    	else if(strcmp(loginList,"<LOGIN_LIST>")==0)
                    	{
                    		fseek(clientList,0,SEEK_END);
                    		long fileSize = ftell(clientList);
                    		fseek(clientList,0,SEEK_SET);
                    		char* list = malloc(fileSize+1);
                    		fread(list,fileSize,1,clientList);
                    		list[fileSize]='\0';
                    		//printf("Client list is %s\n",list);
                    		send(sd,list,strlen(list),0);
                    	}
                    	else if(strcmp(logout,"<LOGOUT>")==0)
                    	{
                    		getpeername(sd , (struct sockaddr*)&serv_addr , (socklen_t*)&clilen);   
                    		printf("user %s logged out , ip %s , port %d \n",usernames[i] , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port));   
                    		//Close the socket and mark as -1in list for reuse 
                    		send(sd,"Exit\n",6,0); 
                    		close( sd );   
                    		free(usernames[i]);
                    		usernames[i]="\n";
                    		FILE* tmp = fopen("tmp.txt","w");
                    		for(int j =0;j<6;j++)
                    		{
                    			fputs(usernames[j],tmp);
                    		}	
                    		fclose(clientList);
                    		fclose(tmp);
                    		remove("clientList.txt");
        	   				rename("tmp.txt","clientList.txt");
        	   				clientList = fopen("clientList.txt","r");
                    		clientSockets[i] = -1;   
                    		count--;
                    	}
                    	else
                    	{
                    		send(sd , buffer , strlen(buffer) , 0 );   
                		}
                	}
                }   
            }   
        }   
    }
	for(int i =0;i<6;i++)
	{
		free(usernames[i]);
	}
	close(sockfd);
	fclose(clientList);
	return 0;
}
