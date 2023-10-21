#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "encDec.h"

int main(int argc, char* argv[])
{
	toUpper(argv[1],argv[2],argv[3]);
}

/*

Converts input string to Upper case and sends to encoder to write to pipeIn

Args:-
	str - decoded string
	len - length of string
	fdIn_One - pipe file descriptor to write to

*/


int toUpper(char* str, char* len,char* fdIn_One)
{
	int length;
	sscanf(len,"%d",&length);
	char upperString[length-1];									//upper case string to be passed as argument
	for(int i =0; i<length;i++)									//loop through characters
	{
		if((int)str[i]>='a' && (int)str[i]<='z')				//if characters are lower case alphabets, convert to upper case
		{
			upperString[i]=toupper(str[i]);
		}
		else
		{
			upperString[i]=str[i];
		}
	}
	str=NULL;
	FILE *fp;
	fp = fopen("data.outf","a");								
	fputs(upperString,fp);										//Write upper case string to data.outf
	fclose(fp);
	length = length-1;
	char lenArg[64];
	sprintf(lenArg,"%d",length);								//store length of chars not including NULL into int
	int pid;
	pid=fork();
	char* isCap = "1";											//set isCap = "1" indicating consumer as parent
	if(pid==0)
	{
		execl("encoderService","encoderService",upperString,lenArg,fdIn_One,isCap,NULL);			//passing upperCase string, length of string, fd for writing to pipe, isCap = "1" to indicate capitalization
	}
	else if(pid>0)
	{
		wait(NULL);
	}
	else
	{
		printf("Fork in loop failed \n");
	}
	return 0;
}
