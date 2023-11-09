#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char* argv[])
{
	deframe(argv[1],argv[2],argv[3],argv[4]);
	return 0;
}

/*

Break down data read from socket into 8 character blocks. Check if first 2 blocks = SYN parity bit binary encoded values

Args:-
	inData - data from socket
	fdIn_One - file descriptor to write to
	flag - encoder flag, h for hamming, c for crc
	file - '\n' if called by client, user1user2.txt if called by server
*/

int deframe(char* inData,char* fdIn_One,char* flag,char* file)
{
	char* characters[100]={};
	if(flag[0]=='h')														//set next function argument to hammingDecode or checkRemoveParityService based on flag
	{
		characters[0] = "hammingDecode";
	}
	else
	{
		characters[0] = "checkRemoveParityService";
	}
	characters[1] = fdIn_One;
	characters[2] = file;
	int j =0;
	int k =3;
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
	if(j>1){
		characters[k] = strdup(block);											//to get remainder bits
	}
	if(strcmp(characters[3],"00010110")!=0 || strcmp(characters[4],"00010110")!=0)			//check if first 2 characters are 22 22
	{
		printf("Incorrect syn chars characters[3] = %s chars[4] = %s\n",characters[3],characters[4]);
	}
	int pid;
	pid = fork();
	if(pid==0)
	{
		if(flag[0]=='h')
		{
			execv("hammingDecode",characters);								//call hammingDecode if flag is h
		}
		else
		{
			execv("checkRemoveParityService",characters);					//call checkRemoveParityService if flag is c
		}
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
