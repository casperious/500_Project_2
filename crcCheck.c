#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"
#include <math.h>

char* crc_gen = "100000100110000010001110110110111"; //x32+ x26+ x23+ x22+ x16+ x12+ x11+ x10+ x8+ x7+ x5+ x4+ x2+ x + 1 10000010 01100000 10001110 11011011 1
	
int main(int argc, char *argv[])
{
	checkCRC(argv[1],argv[2],argv[3],argv[4]);
	return 0;
}
//Perform XOR on input strings and return result
char* XOR(char* x, char* y)
{
	int len = 33;
	char* retString = calloc(34,sizeof(char));													
	for(int i =0;i<33;i++)
	{
		if(x[i]==y[i])
		{
			retString[i]='0';
		}
		else
		{
			retString[i]='1';
		}
	}
	retString[33]='\0';
	return retString;
}


/*

Performs XOR on full frame with CRC32 gen, and checks remainder to detect error

Args:-
	inData = string to be decoded
	fdIn_One = file descriptor for socket to write to
	flag = flag for hamming or crc
	file = file to write decoded message to, '\n' if called by client, username1username2.txt if called by server
*/
void checkCRC(char* inData, char* fdIn_One, char* flag,char* file)
{
	inData[568]='\0';
	int i =0;
	char*  check = calloc(strlen(inData),sizeof(char));
	for(int k=0;k<strlen(inData);k++)
	{
		check[k] = inData[k];
	}
	check[568]='\0';										//create duplicate of input data
	
	while(i+32<strlen(inData))						
	{
		
		if(check[i]=='0')									//skip first 0's
		{
			i++;
		}
		else
		{
			char* xor_string = calloc(strlen(inData)+1,sizeof(char));
			char* x = calloc(strlen(crc_gen)+1,sizeof(char));
			for(int j = 0;j<strlen(crc_gen);j++)					//extract first 33 characters starting from the first 1 to be XOR'd
			{
				x[j] = check[i+j];
			}
			x[33]='\0';
			for(int j =0;j<i;j++)
			{
				xor_string[j]='0';									//set first i values to 0 for next string
			}
			char* retstring = XOR(x,crc_gen);						//get XOR result
			free(x);
			x=NULL;
			for(int j =0;j<33;j++)
			{
				xor_string[i+j] = retstring[j];						//set next 33 values to result from XOR
			}
			free(retstring);
			retstring = NULL;
			for(int j = i+33;j<strlen(check);j++)
			{
				xor_string[j] = check[j];							//set remaining values to previous string values
			}
			xor_string[strlen(inData)]='\0';
			for(int j =0;j<strlen(check);j++)
			{
				check[j] = xor_string[j];							//set duplicate to next string
			}
			free(xor_string);
			xor_string = NULL;
			i++;
		}
	}
	
	int j =0;
	while(j<strlen(check))											//if remainder is not 0. Error detected
	{
		if(check[j]=='1')
		{
			printf("\n----------------------\nData received has been corrupted during transmission\n-----------------------------------\n");
			break;
		}
		j++;
	}
	if(j==strlen(check))
	{
	}
	free(check);
	char* send = calloc(strlen(inData)-strlen(crc_gen)+2,sizeof(char));
	for(int i =0;i<strlen(inData)-strlen(crc_gen)+1;i++)				//extract original data without remainder
	{
		send[i]=inData[i];
	}
	send[strlen(send)]='\0';
	check=NULL;
	int pid;
	pid = fork();
	if(pid==0)
	{
		execl("deframe","deframe",send,fdIn_One,flag,file,NULL);		//send extracted data to deframe
	}
	else if (pid>0)
	{
		wait(NULL);
	}
	else
	{
		printf("Failed fork");
	}
	return;

}
