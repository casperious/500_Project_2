#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
int main(int argc, char *argv[])
{
	int limit = 6;
	int count = 0;
	FILE* clientList;
	clientList = fopen("clientList.txt","w");
	int sockfd, portno;
	int newsockfdList[6];
	socklen_t clilen;
	char buffer[256];
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
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	while(count<limit)
	{	listen(sockfd,5);
		clilen = sizeof(cli_addr);
		newsockfdList[count] = accept(sockfd,
		(struct sockaddr *) &cli_addr,&clilen);
		int newsock = newsockfdList[count];
		count++;
		if (newsock < 0)
			error("ERROR on accept");
		while(1)
		{
			bzero(buffer,256);
			n = read(newsock,buffer,255);
			printf("%d",strcmp(buffer,"Exit\n"));
			if(strcmp(buffer,"Exit\n")==0)
			{
				printf("Read Exit closing server\n");
				n = write(newsock,"Exit\n",5);
				bzero(buffer,256);
				n = read(newsock,buffer,4);
				if(strcmp(buffer,"Done")==0)
				{
					close(newsock);
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
				n = write(newsock,"I got your message",18);
				if (n < 0)
					error("ERROR writing to socket");
			}
		}
	}
	//close(newsockfd);
	close(sockfd);
	fclose(clientList);
	return 0;
}
