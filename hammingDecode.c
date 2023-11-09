#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"
#include <math.h>

int main(int argc, char *argv[])
{
	removeHamming(argv);
	return 0;
}

/*

Decode hamming encoded data and check for errors. Correct error if present

Args:-
	characters - blocks of data
*/
void removeHamming(char** characters)		//char *inData, char* fdOut_One,char* isCap
{
	char* inData=calloc(69*8,sizeof(char));
	int k =0;
	for(int i = 5;i<100;i++)				//convert blocks of data to one long string. This was due to Project 1 implementation
	{
		if(characters[i]==NULL)
		{
			break;
		}
		for(int j =0;j<strlen(characters[i]);j++)
		{
			if(characters[i][j]!='1' && characters[i][j]!='0')
			{
				break;
			}
			inData[k]=characters[i][j];
			k++;
		}
	}	
	inData[k]='\0';
	int errPos = 0;
	int len = strlen(inData);
	int numParity = 0;
	while(pow(2,numParity)<len)				//calculate number of parity bits
	{
		numParity+=1;
	}
	int i =0;
	while(i<numParity)
	{
		
			int numSkip = pow(2,i);			//get number of bits to skip
			int up = 0;
			int down = 0;
			int parity = 0;
			for(int j =numSkip-1;j<len;j++)
			{
				 if(up<numSkip)
				 {
				 	down =0;
				 	if(j==numSkip-1)		//skip
				 	{
				 	}
				 	if(inData[j]=='1' && j!=numSkip-1)	//count parity
				 	{ 
				 		
				 		parity++;
					}
					up++;
				 }
				 else
				 {
				 	down++;
				 	if(down>=numSkip)
				 	{
				 		up=0;
				 	}
				 
				 }
			}
			if(parity%2==1 && inData[numSkip-1]!='1')							//if odd 1's, then parity bit = 1. if parity is incorrect, add parity bit position
			{
				errPos+=numSkip;
			}
			else if(parity%2==0 && inData[numSkip-1]!='0')						//if even 1's, then parity bit = 0. if parity is incorrect, add parity bit position
			{
				errPos+=numSkip;
			}
		i+=1;
	}
	if(errPos>0){																//if error detected, print position and correct it
		printf("\n --------------------------\n Bit at %d is corrupted \n",errPos);
		printf("\nFlipping bit at errPos %d\n --------------------------------------\n",errPos);
		if(inData[errPos-1]=='1')
		{
			inData[errPos-1]='0';
		}
		else
		{
			inData[errPos-1]='1';
		}
	}
	char* decoded = calloc(len-numParity,sizeof(char));
	int j =0;
	for(i =0;i<len;i++)				
	{
		if(ceil(log2(i+1))==floor(log2(i+1)))		//ignore parity bits at powers of 2
		{
		}
		else
		{
			decoded[j]=inData[i];					//add rest of data to decoded string
			j+=1;
			
		}	
	
	}
	decoded[len-numParity] = '\0';
	//build back into blocks and send to checkRemove
	char* send[100]={"checkRemoveParityService",characters[1],characters[2],characters[3],characters[4]};
	int x =0;
	int y =5;
	char block[9]="00000000";
	for(int i =0;i<strlen(decoded);i++)									//loop through decoded and split into 8 char blocks
	{
		if(decoded[i]=='\0')
		{
			break;
		}
		if(x<=7)
		{
			block[x]=decoded[i];
			x++;
		}
		else
		{
			block[x]='\0';
			send[y]=strdup(block);									//save block into characters[k]
			x = 0;
			block[x]=decoded[i];
			x++;
			y++;
		}
	}
	free(inData);
	free(decoded);
	int pid;
	pid = fork();
	if(pid==0)
	{	
		execv("checkRemoveParityService",send);						//send blocks of data to checkRemoveParityService
	}
	else if(pid>0)
	{
		wait(NULL);
	}
	else
	{
		printf("fork in hammingDecode failed\n");
	}
	
}
