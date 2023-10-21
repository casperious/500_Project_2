#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char *argv[])
{
	buildFrame(argv[1],argv[2],argv[3]);
	return 0;
}

/*
Hardcode syn characters binary and prepend to inData. Write finished frame to data.binf if isCap = "0", or data.chck if isCap="1". Then write to pipe fd sent in arg

Args:-
	inData - 8 character, parity bit, binary encoded string
	fdOut_One - pipe file descriptor to write to
	isCap - "0" if parent is producer, "1" if parent is consumer

*/

void buildFrame(char *inData,char* fdOut_One,char* isCap){
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
	write(fdOut,frame,sizeof(frame));						//write to pipe, be it fdOut or fdIn
	return;
}
