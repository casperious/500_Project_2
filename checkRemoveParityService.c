#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char* argv[])
{
	checkRemoveParity(argv);
	return 0;
}

/*

Check parity for each 8 character block. If parities are correct, remove the bit and make 7 char blocks instead.

Args:-
	characters - list of characters with characters[0] is service name, chars[1] is fdIn[1], chars[2] and [3] are syn, chars[4] is numChars
	
*/
int checkRemoveParity(char** characters)		
{
	//printf("In checkRemPar  %s\n",characters[4]);
	int numCharsNumOnes = 0;			
	if(characters[5][0]=='0')					//checking parity for numChars
	{
		for(int i =1;i<8;i++)
		{
			if(characters[5][i]=='1')
			{
				numCharsNumOnes++;
			}
		}
		if(numCharsNumOnes%2==0)
		{
			printf("Num chars has wrong parity. Corrupted Data\n");
		}	
	}
	else
	{
	//printf("First char is 1\n");
		for(int i =1;i<8;i++)
		{
			if(characters[5][i]=='1')
			{
				numCharsNumOnes++;
			}
		}
		if(numCharsNumOnes%2==1)
		{
			printf("Num chars has wrong parity. Corrupted Data\n");
		}	
	}
	//remove first bit in characters[4]
	char lenBin[7]="";
	memcpy(lenBin,&characters[5][1],7);
	int numChars = (int)strtol(lenBin,NULL,2);								//get number of characters in string
	//printf("NumChars is %d\n",numChars);
	char* removedBitsChars[numChars+4];										//+1 for decoder, +1 for fdIn[1], +1 for file, +1 for null. Argument string
	int len = sizeof(removedBitsChars)/sizeof(*removedBitsChars);			//length of newly declared char**
	//printf("len is %d\n",len);
	removedBitsChars[0]="decoder";
	removedBitsChars[1]=characters[1];
	removedBitsChars[2] = characters[2];
	//printf("file in remove parity is %s\n",removedBitsChars[2]);
	int k =3;
	for(int i = 6;i<=numChars+5;i++)										//loop through 8 character blocks in argument passed
	{	
		if(characters[i]==NULL)
		{
			//printf("Null encountered, breaking\n");
			break;
		}
		//printf("At %s\n",characters[i]);
		char block[8];
		if(characters[i][0]=='0')											//check for odd parity
		{
			int numOnes = 0;
			for(int j=1;j<strlen(characters[i]);j++)
			{	
				if(characters[i][j]=='1')
				{
					numOnes++;
				}	
			}
			if(numOnes%2==0)
			{
				printf("Incorrect parity. Corrupted Data\n");
			}
			strncpy(block,characters[i]+1,7);								//save 7 characters to block, excluding parity bit
		}
		else
		{
			int numOnes = 0;
			for(int j=1;j<8;j++)
			{	
				if(characters[i][j]=='1')
				{
					numOnes+=1;
				}
			}
			if(numOnes%2==1)
			{
				printf("Incorrect parity. Corrupted Data\n");
			}
			strncpy(block,characters[i]+1,7);
		}	
		removedBitsChars[k]=strdup(block);									//save block to argument string
		//printf("%s\n",removedBitsChars[k]);
		k++;
	}
	removedBitsChars[len]=NULL;											//set last argument to NULL
	//printf("Before fork\n");
	int pid;
	pid = fork();
	if(pid==0)
	{
		execv("decoder",removedBitsChars);									//send argument string to decoder
	}
	else if (pid>0)
	{
		wait(NULL);
	}
	else
	{
		printf("Fork in removeParity failed\n");
		return -1;
	}
	return 0;
}
