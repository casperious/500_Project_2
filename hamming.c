#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"
#include <math.h>
int main(int argc, char *argv[])
{
	addHamming(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
	return 0;
}
/*

Encode frame using hamming encoding. Add 0 as parity bits in p1,p2,p4 and all powers of 2 within string length, check even parity and set parity bit values accordingly

Args:-
	inData - data from socket
	fdOut_One - socket to write to
	isCap - flag to indicate to add error. "1" if error to be added, "0" if not.
	flag - encoder flag, h for hamming, c for crc
	username - sender username
	to - receiver username
*/

void addHamming(char *inData, char* fdOut_One,char* isCap,char* flag, char* username, char* to)
{
	
	int len = strlen(inData);
	int numParityBits = 1;
	while(1)															//calculate #parity bits by checking if 2^r >= m + r + 1 [r = #parity bits, m = len of input]
	{
		int bitNum = pow(2,numParityBits);
		int check = len + numParityBits + 1;
		if(bitNum<check)
		{
			numParityBits+=1;
		}
		else
		{
			break;
		}
	}
	int outLen = len+numParityBits+1;
	char* outData = calloc(outLen+1,sizeof(char));
	for(int i =0;i<len+numParityBits;i++)
	{
		outData[i]='0';
	}
	outData[strlen(outData)]='\0';
	int k =0;
	for(int j =1;j<outLen;j++)
	{
		if(ceil(log2(j))==floor(log2(j)))				//power of 2
		{
		}
		else
		{
			outData[j-1]=inData[k];
			k++;
		}
	}
	
	
	
	for(int i =0;i<numParityBits;i++)
	{
		int numSkip = pow(2,i);							//get number of bits to skip
		int up = 0;
		int down = 0;
		int parity = 0;
		for(int j =numSkip-1;j<outLen-1;j++)
		{
			 if(up<numSkip)
			 {
			 	
			 	down =0;
			 	if(j==numSkip-1)						//dont count parity
			 	{
			 	}
			 	if(outData[j]=='1' && j!=numSkip-1)
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
		if(parity%2==1)							//if odd 1's, then parity bit = 1
		{
			outData[numSkip-1]='1';
		}
		else										//if even 1's, then parity bit = 0
		{
			outData[numSkip-1]='0';
		}
	
	}
	int currPid = getpid();
	srand(currPid);												//use current pid as seed
	int random = rand();
	int idx = random%outLen;									//get index to be flipped	
	
	if(isCap[0]=='1')											//generate error if isCap is 1
	{
		printf("Inserting error at index %d\n",idx);
		if(outData[idx]=='1')
		{
			outData[idx]='0';
		}
		else
		{
			outData[idx]='1';
		}
	}
	int pid;
	pid = fork();
	if(pid==0)
	{
		execl("buildFrameService","buildFrameService",outData,fdOut_One,isCap,flag,username,to,NULL);	//call buildFrameService
	}
	else if(pid>0)
	{
		wait(NULL);
	}

	
	free(outData);


}
