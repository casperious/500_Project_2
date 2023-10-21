#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char* argv[])
{
	deframe(argv[1],argv[2]);
	return 0;
}

/*

Break down data read from pipe into 8 character blocks. Check if first 2 blocks = SYN parity bit binary encoded values

Args:-
	inData - data from pipe
	fdIn_One - file descriptor to write to

*/

int deframe(char* inData,char* fdIn_One)
{
	char* characters[69]={"checkRemoveParityService",fdIn_One};				//intialize argument string for execv with service name, pipe fd
	int j =0;
	int k =2;
	char block[9]="00000000";
	for(int i =0;i<strlen(inData)+1;i++)									//loop through inData and split into 8 char blocks
	{
		
		if(j<=7)
		{
			block[j]=inData[i];
			j++;
		}
		else
		{
			characters[k]=strdup(block);									//save block into characters[k]
			j = 0;
			block[j]=inData[i];
			j++;
			k++;
		}
	}
	if(strcmp(characters[2],"00010110")!=0 || strcmp(characters[3],"00010110")!=0)			//check if first 2 characters are 22 22
	{
		printf("Incorrect syn chars characters[2] = %s chars[3] = %s\n",characters[2],characters[3]);
		return -1;
	}
	int pid;
	pid = fork();
	if(pid==0)
	{
		execv("checkRemoveParityService",characters);						//call checkRemoveParityService with arg string
	}
	else if(pid>0)
	{
		wait(NULL);
	}
	else
	{
		printf("fork in deframe failed\n");
	}
	
	return 0;

}
