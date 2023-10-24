#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"
#include <math.h>

int main(int argc, char *argv[])
{
	removeHamming(argv[1],argv[2],argv[3]);
	return 0;
}

void removeHamming(char *inData, char* fdOut_One,char* isCap)
{
	int errPos = 0;
	int len = strlen(inData);
	//printf("%d\n",len);
	int numParity = 0;
	while(pow(2,numParity)<len)
	{
		numParity+=1;
	}
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
	printf("%s\n",decoded);
	free(decoded);
	
}
