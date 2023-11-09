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
	addCRC(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
	return 0;
}

char* XOR(char* x, char* y)
{
	int len = 33;
	char* retString = calloc(34,sizeof(char));													
	for(int i =0;i<33;i++)
	{
		if(x[i]==y[i])									//if same bit, result is 0
		{
			retString[i]='0';
		}
		else											//if bits differ, result is 1
		{
			retString[i]='1';
		}
	}
	retString[33]='\0';
	return retString;
}

/*

Performs XOR on full frame with CRC32 gen, and adds remainder to frame to send

Args:-
	inData = string to be encoded
	fdOut_One = file descriptor for socket to write to
	isCap = flag to check if error is to be generated. "1" if error, "0" if none
	flag = flag for hamming or crc
	username = sender username
	to = receiver username
*/

void addCRC(char* inData, char* fdOut_One, char* isCap,char* flag, char* username, char* to)
{
	int len = strlen(inData);		
	int outLen = len+strlen(crc_gen)-1;							//output length = input+genLength-1
	char* extendedData = malloc(outLen+1);
	for(int i =0;i<outLen;i++)
	{
		if(i<len)
		{
			extendedData[i]=inData[i];
		}
		else													//add 32 0's as padding for division
		{
			extendedData[i]='0';
		}
	}
	extendedData[outLen]='\0';
	int i =0;
	while(i<len)						
	{
		if(extendedData[i]=='0')
		{
			i++;
		}
		else
		{
			char* xor_string = calloc(outLen+1,sizeof(char));
			char* x = calloc(strlen(crc_gen)+1,sizeof(char));
			for(int j = 0;j<strlen(crc_gen);j++)				//get first 33 characters from i for XOR
			{
				x[j] = extendedData[i+j];
			}
			x[33]='\0';
			for(int j =0;j<i;j++)
			{
				xor_string[j]='0';								//set first i values of next string to be divided to 0
			}
			char* retstring = XOR(x,crc_gen);					//get XOR return value
			free(x);
			x=NULL;
			for(int j =0;j<33;j++)
			{
				xor_string[i+j] = retstring[j];					//set result from XOR into next string
			}
			free(retstring);
			retstring = NULL;
			for(int j = i+33;j<strlen(extendedData);j++)		//set remaining bits from previous string to next string
			{
				xor_string[j] = extendedData[j];
			}
			xor_string[outLen]='\0';
			for(int j =0;j<strlen(extendedData);j++)			//set extendedData to xor_string for next iteration
			{
				extendedData[j] = xor_string[j];
			}
			free(xor_string);
			xor_string = NULL;
			i++;
		}
	}
	char* rem = malloc(strlen(crc_gen));
	int j =0;
	for(int i =strlen(inData);i<outLen;i++)						//get remainder from calculations
	{
		rem[j]=extendedData[i];
		j++;
	}
	rem[32]='\0';				
	char* encodedString = malloc(outLen+1);
	for(int i =0;i<outLen;i++)									//append remainder to frame passed as argument
	{
		if(i<strlen(inData))
		{
			encodedString[i]=inData[i];
		}
		else
		{
			encodedString[i] = rem[i-strlen(inData)];
		}
	}	
	int currPid = getpid();
	srand(currPid);												//use current pid as seed
	int random = rand();
	int idx = random%outLen;									//get index to be flipped	
	if(isCap[0]=='1')											//generate error if required
	{
		printf("Inserting error at index %d\n",idx);
		if(encodedString[idx]=='1')
		{
			encodedString[idx]='0';
		}
		else
		{
			encodedString[idx]='1';
		}
	}
	encodedString[outLen]='\0';
	char* toStart = "<TO>";
	char* toEnd = "</TO>";
	char* toString = calloc(18,sizeof(char));
	strcat(toString,toStart);
	strcat(toString,to);
	strcat(toString,toEnd);
	toString[17]='\0';									//set <TO> tag
	char* fromStart = "<FROM>";
	char* fromEnd = "</FROM>";
	char* fromString = calloc(22,sizeof(char));
	strcat(fromString, fromStart);
	strcat(fromString,username);
	strcat(fromString,fromEnd);
	fromString[21]='\0';								//set <FROM> tag
	char* encodeStart = "<ENCODE>";
	char* encodeEnd = "</ENCODE>";
	char* encodeString = calloc(26,sizeof(char));
	strcat(encodeString, encodeStart);
	strcat(encodeString,flag);
	strcat(encodeString,encodeEnd);
	encodeString[25]='\0';								//set <ENCODE> tag
	char* msgStart = "<MSG>";
	char* msgEnd = "</MSG>";
	char* bodyStart = "<BODY>";
	char* bodyEnd = "</BODY>";
	char* finalFrame = calloc(90+sizeof(encodedString)+1,sizeof(char));
	strcat(finalFrame,msgStart);
	strcat(finalFrame,fromString);
	strcat(finalFrame,toString);
	strcat(finalFrame,encodeString);
	strcat(finalFrame,bodyStart);
	strcat(finalFrame,encodedString);
	strcat(finalFrame,bodyEnd);
	strcat(finalFrame,msgEnd);
	finalFrame[strlen(finalFrame)] = '\0';			//set full <MSG> tag message
	int fdOut;
	sscanf(fdOut_One,"%d",&fdOut);									//extract fd to write to
	write(fdOut,finalFrame,strlen(finalFrame));						//write to socket
	free(toString);
	free(fromString);
	free(encodeString);
	free(extendedData);
	free(rem);
	free(encodedString);
	encodedString = NULL;
	rem=NULL;
	extendedData=NULL;
}
