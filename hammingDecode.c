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

void removeHamming(char** characters)		//char *inData, char* fdOut_One,char* isCap
{
	printf("In hamming decode\n");
	char* inData=calloc(69*8,sizeof(char));
	int k =0;
	for(int i = 4;i<100;i++)
	{
		if(characters[i]==NULL)
		{
			printf("breaking at %d\n",i);
			break;
		}
		printf("Looking at %s at %d\n",characters[i],i);
		for(int j =0;j<strlen(characters[i]);j++)
		{
			if(characters[i][j]!='1' && characters[i][j]!='0')
			{
				break;
			}
			inData[k]=characters[i][j];
			printf("%c",inData[k]);
			k++;
		}
		
		printf("\n");
	}
	printf("inData is now\n");
	inData[k]='\0';
	printf("%s\n",inData);
	int errPos = 0;
	int len = strlen(inData);
	printf("%d\n",len);
	int numParity = 0;
	while(pow(2,numParity)<len)
	{
		numParity+=1;
	}
	printf("numParity is %d\n",numParity);
	int i =0;
	while(i<numParity)
	{
		
			int numSkip = pow(2,i);
			//printf("checking %d and skipping %d\n",numSkip,numSkip);
			int up = 0;
			int down = 0;
			int parity = 0;
			for(int j =numSkip-1;j<len;j++)
			{
				 if(up<numSkip)
				 {
				 	//printf("checking %d is %c\n",j,inData[j]);
				 	down =0;
				 	if(j==numSkip-1)
				 	{
				 		//printf("not adding i to parity\n");
				 	}
				 	if(inData[j]=='1' && j!=numSkip-1)
				 	{ 
				 		
				 		parity++;
					}
					up++;
				 }
				 else
				 {
				 	//printf("Skipping %d\n",j);
				 	down++;
				 	if(down>=numSkip)
				 	{
				 		up=0;
				 	}
				 
				 }
			}
			if(parity%2==1 && inData[numSkip-1]!='1')							//if odd 1's, then parity bit = 1
			{
				printf("Incorrect parity %d at %d\n",parity,numSkip);
				errPos+=numSkip;
			}
			else if(parity%2==0 && inData[numSkip-1]!='0')						//if even 1's, then parity bit = 0
			{
				printf("Incorrect parity %d at %d\n",parity,numSkip);
				errPos+=numSkip;
			}
		i+=1;
	}
	if(errPos>0){
		printf("Flipping bit at errPos %d\n",errPos);
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
		if(ceil(log2(i+1))==floor(log2(i+1)))
		{
			printf("parity bit %c at %d being removed\n",inData[i],i);
		}
		else
		{
			decoded[j]=inData[i];
			j+=1;
		}	
	
	}
	decoded[len-numParity] = '\0';
	//build back into blocks and send to checkRemove
	printf("chars[1] is %s\n",characters[1]);
	char* send[69]={"checkRemoveParityService",characters[1],characters[2],characters[3]};
	int x =0;
	int y =4;
	char block[9]="00000000";
	for(int i =0;i<strlen(decoded)+1;i++)									//loop through inData and split into 8 char blocks
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
			send[y]=strdup(block);									//save block into characters[k]
			x = 0;
			block[x]=decoded[i];
			x++;
			y++;
		}
	}
	printf("%s\n",decoded);
	free(inData);
	free(decoded);
	for(int i =0;i<69;i++)
	{
		if(send[i]==NULL)
		{
			break;
		}
		printf("%s\n",send[i]);
	}
	int pid;
	printf("Calling fork\n");
	pid = fork();
	if(pid==0)
	{	
		printf("sending to removeParity\n");
		execv("checkRemoveParityService",send);	
	}
	else if(pid>0)
	{
		printf("Waiting in hamming decode\n");
		wait(NULL);
	}
	else
	{
		printf("fork in hammingDecode failed\n");
	}
	
}
