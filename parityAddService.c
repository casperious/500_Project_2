#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char *argv[])
{
	addParity(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
	return 0;
}

/*

Go through all characters and break them into chunks of 7. check for odd parity, and add a 0 or 1 to the MSB

Args:-
	inData - 7 character binary encoded data, length + chars
	fdOut_One - socket file descriptor to write to
	isCap - 0 if no error generation, 1 if error generation
	flag= flag for hamming or crc
	username = sender username
	to = receiver username
*/
void addParity(char *inData, char* fdOut_One,char* isCap,char* flag, char* username, char* to)
{
	char outData[1025] = "";								//parity bit added, binary encoded result string
	int j =1;
	char buffer[9] = "0";
	for(int i =0;i<strlen(inData)+1;i++)
	{
		if(j<=7)
		{
			buffer[j] = inData[i];							//build 7 char blocks
			j++;
			
		}
		else
		{
			j = 1;
			i--;
			int numOnes = 0;
			char parityBit='\0';
			for(int k = 1;k<strlen(buffer);k++)			//check for # of 1's
			{
				if(buffer[k]=='1')
				{
					numOnes++;
				}
			}
			if(numOnes%2==0)							//if even 1's, then parity bit = 1
			{
				parityBit='1';
			}
			else										//if odd 1's, then parity bit = 0
			{
				parityBit='0';
			}
			buffer[0] = parityBit;
			strncat(outData,buffer,8);					//add 8 char block to outData
		}
	}
	int pid;
	pid = fork();
	if(pid==0)
	{
		if(flag[0]=='h')
		{
			execl("hamming","hamming",outData,fdOut_One,isCap,flag,username,to,NULL);
		}
		else
		{
			execl("buildFrameService","buildFrameService",outData,fdOut_One,isCap,flag,username,to,NULL);		//call buildFrameService with 8 char, parity bit, binary encoded string, fd to write to, isCap, encoder flag, sender and reciever usernames
		}
	}
	else if(pid>0)
	{
		wait(NULL);
	}

}
