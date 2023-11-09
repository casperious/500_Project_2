#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char *argv[])
{
	buildFrame(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
	return 0;
}

/*
Hardcode syn characters binary and prepend to inData. Send finished frame to socket if encoder flag is h, send to crcAdd for encoding if flag is c

Args:-
	inData - 8 character, parity bit, binary encoded string, either hamming encoded, or to be sent to crc for encoding
	fdOut_One - socket file descriptor to write to
	isCap - "0" if no error is generated, "1" if error is generated
	flag - h for hamming, c for crc
	username - sender username
	to - receiver username
*/

void buildFrame(char *inData,char* fdOut_One,char* isCap,char* flag, char* username, char* to){
	int fdOut;
	sscanf(fdOut_One,"%d",&fdOut);									//extract fd to write to
	char syn[8] = "00010110";										//hard code parity bit binary encoded SYN (22)
	char frame[1025]="";											//final frame to be sent
	strncat(frame,syn,8);											//add 2222 to frame	
	strncat(frame,syn,8);
	strncat(frame,inData,strlen(inData));							//concat inData to 2222
	
	if(flag[0]=='h')
	{
		char* toStart = "<TO>";
		char* toEnd = "</TO>";
		char* toString = calloc(18,sizeof(char));
		strcat(toString,toStart);
		strcat(toString,to);
		strcat(toString,toEnd);
		toString[17]='\0';											//Create <TO> tag
		char* fromStart = "<FROM>";
		char* fromEnd = "</FROM>";
		char* fromString = calloc(22,sizeof(char));
		strcat(fromString, fromStart);
		strcat(fromString,username);
		strcat(fromString,fromEnd);
		fromString[21]='\0';										//create <FROM> tag
		char* encodeStart = "<ENCODE>";
		char* encodeEnd = "</ENCODE>";
		char* encodeString = calloc(26,sizeof(char));
		strcat(encodeString, encodeStart);
		strcat(encodeString,flag);
		strcat(encodeString,encodeEnd);
		encodeString[25]='\0';										//create <ENCODE> tag
		char* msgStart = "<MSG>";
		char* msgEnd = "</MSG>";
		char* bodyStart = "<BODY>";
		char* bodyEnd = "</BODY>";
		char* finalFrame = calloc(90+sizeof(frame)+1,sizeof(char));	
		strcat(finalFrame,msgStart);
		strcat(finalFrame,fromString);
		strcat(finalFrame,toString);
		strcat(finalFrame,encodeString);
		strcat(finalFrame,bodyStart);
		strcat(finalFrame,frame);
		strcat(finalFrame,bodyEnd);
		strcat(finalFrame,msgEnd);
		finalFrame[strlen(finalFrame)] = '\0';						//create full <MSG> tag to be sent
		write(fdOut,finalFrame,strlen(finalFrame));					//send <MSG> message to socket
		free(toString);
		free(fromString);
		free(encodeString);
		free(finalFrame);
	}
	else
	{
		int pid;
		pid = fork();
		if(pid==0)
		{
			execl("crcAdd","crcAdd",frame,fdOut_One,isCap,flag,username,to,NULL);	//send built frame only with SYN chars added, no tags, to crcAdd 
		}
		else if(pid>0)
		{
			wait(NULL);	
		}
	}
	return;
}
