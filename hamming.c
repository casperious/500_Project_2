#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"
#include <math.h>
int main(int argc, char *argv[])
{
	addHamming(argv[1],argv[2],argv[3]);
	return 0;
}

void addHamming(char *inData, char* fdOut_One,char* isCap)
{
	
	int len = strlen(inData);
	printf("length is %d\n",len);
	int numParityBits = 1;
	while(1)																//calculate #parity bits by checking if 2^r >= m + r + 1 [r = #parity bits, m = len of input]
	{
		int bitNum = pow(2,numParityBits);
		int check = len + numParityBits + 1;
		//printf("%d\n",bitNum);
		if(bitNum<check)
		{
			numParityBits+=1;
		}
		else
		{
			break;
		}
	}
	printf("Number of parity bits to add is %d\n",numParityBits);
	int outLen = len+numParityBits+1;
	printf("outLen is %d\n",outLen);
	printf("2^r is %f\n",pow(2,numParityBits));
	if(pow(2,numParityBits)>=outLen)
	{
		printf("Correct\n");
	}
	char* outData = calloc(outLen,sizeof(char));
	printf("%s\n",outData);
	for(int i =0;i<len+numParityBits;i++)
	{
		outData[i]='0';
	}
		
	for(int i =0;i<outLen;i++)
	{
		printf("%c",outData[i]);
	}
	
	printf(" of length %ld\n",strlen(outData));
	int k =0;
	//for(int i =0;i<numParityBits;i++)
	//{
	//	int idx = pow(2,i);
	for(int j =1;j<outLen;j++)
	{
		if(ceil(log2(j))==floor(log2(j)))				//power of 2
		{
			printf("Skipping %c at idx %d\n",outData[j-1],j-1);
		}
		else
		{
			printf("outData[%d] is %c\n",j-1,outData[j-1]);
			outData[j-1]=inData[k];
			printf("outData[%d] is now %c\n",j-1,outData[j-1]);
			k++;
		}
	}
	
	//}
	printf("data with 0's in powers of 2\n");
	for(int i =0;i<outLen;i++)
	{
		printf("%c",outData[i]);
	}
	printf(" with length %ld\n",strlen(outData));
	
	printf("checking and setting parity bits\n");
	
	for(int i =0;i<numParityBits;i++)
	{
		int numSkip = pow(2,i);
		printf("checking %d and skipping %d\n",numSkip,numSkip);
		int up = 0;
		int down = 0;
		int parity = 0;
		for(int j =i;j<outLen;j++)
		{
			 if(up<numSkip)
			 {
			 	printf("checking %d is %c\n",j,outData[j]);
			 	down =0;
			 	if(outData[j]=='1' && j!=i)
			 	{
			 		parity++;
				}
				up++;
			 }
			 else
			 {
			 	printf("Skipping %d\n",j);
			 	down++;
			 	if(down>=numSkip)
			 	{
			 		up=0;
			 	}
			 
			 }
		}
		if(parity%2==1)							//if odd 1's, then parity bit = 1
		{
			outData[numSkip-1]='1';
		}
		else										//if even 1's, then parity bit = 0
		{
			outData[numSkip-1]='0';
		}
	
	}
	for(int i =0;i<outLen;i++)
	{
		printf("%c",outData[i]);
	}
	printf(" with length %ld\n",strlen(outData));				//001011101111 vs 111011101111
	
	free(outData);


}
