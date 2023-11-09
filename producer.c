#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"



int main(int argc, char *argv[]){
	producer(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
	return 0;
}
/*

Recieve raw text input and break into chunks of 64 characters to send as frames to server

Args:-
	port - socket number
	fl - encoder flag, h for hamming, c for crc
	buffer - text input
	username - sender username
	to - receiver username
	frameNum - frame number to include error
*/

int producer(char* port, char* fl, char* buffer, char* username, char* to, char* frameNum)
{
	
	char str[65];															//64 character chunks
	
	int pid;
	
	char* flag="";		
	
	
	int portno = atoi(port);
	int frameNo = atoi(frameNum);
	
	if(fl[0]=='h')															//set argument for future methods
	{
		flag="h\0";
	}
	else
	{
		flag="c\0";
	}
	int count = 0;
	char ch;
	
	int newPid;
	int numFrames=0;														//keep track of number of frames to generate error in 3rd frame
	for(int z = 0;z<strlen(buffer);z++)
	{
		ch = buffer[z];
		if(count<64){
			str[count]=ch;													//build frame of 64 chars
			count++;
			if(count==64)
			{
				numFrames++;												//increase numFrames by 1
				if(numFrames==frameNo)
				{
					newPid=fork();
					if(newPid==0)
					{
						execl("encoderService","encoderService",str,"64",port,"1", flag, username,to,NULL);		//send frame to encoder service with error flag
					}
					else if(newPid>0)
					{
						wait(NULL);
					}
					else
					{
						printf("Fork in error failed\n");
					}
				}
				else{
					newPid = fork();
					if(newPid==0)
					{
						
						execl("encoderService","encoderService",str,"64",port,"0",flag, username,to,NULL);	//send frame to encoder service with no error flag
					}
					else if(newPid>0)
					{
						wait(NULL);
					}
					else
					{
						printf("Fork in loop failed \n");
					}
				}
			}
		}
		else
		{
			memset(str,'\0',65);														//wipe str
			str[0]=ch;																	//set str = newly read char
			count=1;
		}
	}
	char countStr[64];
	sprintf(countStr,"%d",count);														//store length of last block into countStr
	int status,options;
	if(count>0){
		numFrames++;
		if(numFrames==frameNo)																
		{
			pid = fork();
			if(pid==0)
			{
				execl("encoderService","encoderService",str,countStr,port,"1",flag, username,to,NULL);	//send frame to encoder service with error flag
			}
			else if(pid>0)
			{
				wait(NULL);
			}
			else
			{
				printf("Failed fork in error\n");
			}
		}
		else																			
		{
			pid=fork();
			if(pid==0){
				execl("encoderService","encoderService",str,countStr,port,"0",flag, username,to,NULL);	//send frame to encoder service with no error flag
			}
			else if(pid>0){
				wait(NULL);
			}
			else
			{
				printf("No fork \n");
			}
		}
	}
	
	return 0;
}
