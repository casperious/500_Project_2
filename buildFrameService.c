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
Hardcode syn characters binary and prepend to inData. Write finished frame to data.binf if isCap = "0", or data.chck if isCap="1". Then write to pipe fd sent in arg

Args:-
	inData - 8 character, parity bit, binary encoded string
	fdOut_One - pipe file descriptor to write to
	isCap - "0" if parent is producer, "1" if parent is consumer

*/

void buildFrame(char *inData,char* fdOut_One,char* isCap,char* flag, char* username, char* to){
	int fdOut;
	sscanf(fdOut_One,"%d",&fdOut);									//extract fd to write to
	char syn[8] = "00010110";										//hard code parity bit binary encoded SYN (22)
	char frame[1025]="";											//final frame to be sent
	strncat(frame,syn,8);											//add 2222 to frame	
	strncat(frame,syn,8);
	strncat(frame,inData,strlen(inData));							//concat inData to 2222
	FILE *fp;
	if(strcmp(isCap,"0")==0)										//not capitalized. Producer data flow. 			
	{
		fp = fopen("data.binf","a");								//write to data.binf
		fputs(frame,fp);
	}
	else															//capitalized, called from consumer
	{
		fp = fopen("data.chck","a");								//write to data.chck
		fputs(frame,fp); 	
	}
	fclose(fp);												//close file written to
	//printf("Writing %s of length %ld to %d\n",frame,strlen(frame),fdOut);
	if(flag[0]=='h')
	{
		char* toStart = "<TO>";
		char* toEnd = "</TO>";
		char* toString = calloc(18,sizeof(char));
		strcat(toString,toStart);
		strcat(toString,to);
		strcat(toString,toEnd);
		toString[17]='\0';
		//printf("To is %s\n",toString);
		char* fromStart = "<FROM>";
		char* fromEnd = "</FROM>";
		char* fromString = calloc(22,sizeof(char));
		strcat(fromString, fromStart);
		strcat(fromString,username);
		strcat(fromString,fromEnd);
		fromString[21]='\0';
		//printf("From is %s\n",fromString);
		char* encodeStart = "<ENCODE>";
		char* encodeEnd = "</ENCODE>";
		char* encodeString = calloc(26,sizeof(char));
		strcat(encodeString, encodeStart);
		strcat(encodeString,flag);
		strcat(encodeString,encodeEnd);
		encodeString[25]='\0';
		//printf("Encode is %s\n",encodeString);
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
		finalFrame[strlen(finalFrame)] = '\0';
		//printf("Writing %s to socket\n",finalFrame);
		write(fdOut,finalFrame,strlen(finalFrame));						//write to pipe, be it fdOut or fdIn
		free(toString);
		free(fromString);
		free(encodeString);
		free(finalFrame);
	}
	else
	{
		/*FILE* ptr;
		ptr = fopen("check.inpf","a");
		//printf("Putting %s in check.inpf\n",frame);
		fputs(frame,ptr);
		fputs("\n",ptr);
		fclose(ptr);
		*/
		int pid;
		pid = fork();
		if(pid==0)
		{
			execl("crcAdd","crcAdd",frame,fdOut_One,isCap,flag,username,to,NULL);
		}
		else if(pid>0)
		{
			wait(NULL);	
		}
	}
	return;
}
