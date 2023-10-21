#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

//-1 return is fork failure
//-2 return is pipe failure

int main(){
	FILE* ptr;									//input file pointer
	FILE* binf;									//binf file pointer
	FILE* outf;									//outf file pointer
	FILE* clearDone;							//done file pointer
	FILE* chck;									//chck file pointer
	char str[65];
	
	int pid;
	
	ptr = fopen("data.inpf","r");				//open inpf to read
	binf = fopen("data.binf","w");				//wipe data.binf
	fclose(binf);
	outf = fopen("data.outf","w");				//wipe data.outf
	fclose(outf);
	chck = fopen("data.chck","w");				//wipe data.chck
	fclose(chck);
	if(NULL== ptr){
		printf("File not found \n");
		return(-1);
	}
	
	int count = 0;
	char ch;
	int fdOut[2];									//p=>c
	int fdIn[2];									//c=>p
	int consPid;
	if(pipe(fdOut)==-1)								
	{
		printf("fdOut pipe failed\n");
		return -2;
	}
	if(pipe(fdIn)==-1)
	{
		printf("fdIn pipe failed\n");
		return -2;
	}
	consPid = fork();														//fork to create consumer child process
	if(consPid==0)
	{
		close(fdOut[1]);													//close write of p=>c
		close(fdIn[0]);														//close read of c=>p
		
		char arg1[4];
		sprintf(arg1,"%d",fdOut[0]);
		char arg2[4];
		sprintf(arg2,"%d",fdIn[1]);
		execl("consumer","consumer",arg1,arg2,NULL);						//create consumer, with fdOut[0] to read from, and fdIn[1] to write to
	}
	else if(consPid>0)
	{
		clearDone = fopen("data.done","w");									//wipe data.done
		fclose(clearDone);
		close(fdOut[0]);													//close read of p=>c
		close(fdIn[1]);														//close write of c=>p
	}
	else
	{
		printf("Failed to create consumer fork\n");
		return -1;
	}
	char arg[4];
	sprintf(arg,"%d",fdOut[1]);												//store fdOut[1] in string to pass as arg to other functions through exec
	int newPid;
	int numFrames=0;														//keep track of number of frames to generate error in 3rd frame
	while((ch = getc(ptr))!=EOF)
	{
		if(count<64){
			str[count]=ch;													//build frame of 64 chars
			count++;
			if(count==64)
			{
				numFrames++;												//increase numFrames by 1
				if(numFrames==3)
				{
					newPid=fork();
					if(newPid==0)
					{
						execl("errorService","errorService",str,"64",arg,"0",NULL);		//call error service if numFrames is 3 i.e. 3rd frame
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
						
						execl("encoderService","encoderService",str,"64",arg,"0",NULL);	//if not 3rd frame, then call encoder service with 64 char string, length of 64, fdOut[1], isCap="0"
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
		if(numFrames==3)																//if 3rd frame, then call errorService
		{
			pid = fork();
			if(pid==0)
			{
				execl("errorService","errorService",str,countStr,arg,"0",NULL);
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
		else																			//if not 3rd frame, then call encoderService
		{
			pid=fork();
			if(pid==0){
				execl("encoderService","encoderService",str,countStr,arg,"0",NULL);		
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
	char buff[1025];	
	ssize_t capped;
	close(fdOut[1]);																	//close write end of fdOut. Consumer will stop reading
	int lastPid;
	while((capped=read(fdIn[0],buff,sizeof(buff)))>0)									//read from fdIn till consumer finishes sending capitalized frames to buildFrameService
	{
		char argLast[4];
		sprintf(argLast,"%d",-1);														//set fd arg for decoder = -1, indicating write to data.done instead of pipe
		lastPid=fork();
		if(lastPid==0){
			execl("deframe","deframe",buff,argLast,NULL);
		}
		else if(lastPid>0)
		{
			wait(NULL);
		}
		else
		{
			printf("Last fork\n");
		}
	}
	close(fdIn[0]);																		//close read pipe
	waitpid(lastPid,&status,options);													//wait on decoder to finish writing	
	waitpid(consPid,&status,options);													//wait on consumer process to finish
	printf("finished consumer ending producer\n");
	fclose(ptr);																		//close input file
	return 0;
}
