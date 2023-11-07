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
	int len = strlen(y);
	char* retString = calloc(strlen(crc_gen),sizeof(char));													//hardcoded length of divisor
	for(int i =0;i<strlen(y);i++)
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
	return retString;
}

void addCRC(char* inData, char* fdOut_One, char* isCap,char* flag, char* username, char* to)
{
	int len = strlen(inData);
	//printf("inData is %s and length of indata is %ld\n",inData,strlen(inData));
	int outLen = len+strlen(crc_gen)-1;
	//printf("%d\n",outLen);
	char* extendedData = malloc(outLen+1);
	//printf("%ld\n",strlen(extendedData));
	for(int i =0;i<outLen;i++)
	{
		if(i<len)
		{
			extendedData[i]=inData[i];
		}
		else
		{
			extendedData[i]='0';
		}
		//printf("%c %ld %d\n",extendedData[i],strlen(extendedData),i);
	}
	extendedData[outLen]='\0';
	//printf("-----------------------------------------\n");
	//printf("inData with padded 0's is of length %ld is %s \n",strlen(extendedData),extendedData);
	int i =0;
	while(i+strlen(crc_gen)<outLen)
	{
		//printf("Extended Data is %s\n",extendedData);
		if(extendedData[i]=='0')
		{
			i++;
		}
		else
		{
			//printf("%s len %ld\n",extendedData,strlen(extendedData));
			/*for(int j = 0;j<i;j++)
			{
				printf(" ");
			}*/
			//printf("%s len %ld\n",crc_gen,strlen(crc_gen));
			char* xor_string = calloc(outLen,sizeof(char)+1);
			char* x = calloc(strlen(crc_gen),sizeof(char)+1);
			for(int j = i;j<strlen(crc_gen)+i;j++)
			{
				x[j-i] = extendedData[j];
			}
			x[strlen(crc_gen)]='\0';
			for(int j =0;j<i;j++)
			{
				xor_string[j]='0';
			}
			//xor_string[outLen] = '\0';
			char* retstring = XOR(x,crc_gen);
			free(x);
			x=NULL;
			for(int j =i;j<i+strlen(crc_gen);j++)
			{
				xor_string[j] = retstring[j-i];
			}
			free(retstring);
			retstring = NULL;
			for(int j = i+strlen(crc_gen);j<strlen(extendedData);j++)
			{
				xor_string[j] = '0';
			}
			xor_string[outLen]='\0';
			//printf("XOR_String is %s\n",xor_string);
			for(int j =0;j<strlen(extendedData);j++)
			{
				extendedData[j] = xor_string[j];
			}
			free(xor_string);
			xor_string = NULL;
			i++;
		}
	}
	char* rem = malloc(strlen(crc_gen));
	//printf("extracting remainder from %s\n",extendedData);
	for(int i =strlen(inData);i<outLen;i++)
	{
		rem[i-strlen(inData)]=extendedData[i];
	}
	rem[strlen(crc_gen)-1]='\0';
	//printf("Remainder is %s of length %ld\n",rem,strlen(rem));					//01001001000111011110011101100010 vs 01001001000111011110011101100010
	char* encodedString = malloc(outLen+1);
	for(int i =0;i<outLen;i++)
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
	
	char* toStart = "<TO>";
	char* toEnd = "</TO>";
	char* toString = calloc(18,sizeof(char));
	strcat(toString,toStart);
	strcat(toString,to);
	strcat(toString,toEnd);
	toString[17]='\0';
	//printf("To is %s\n",toString);
	char* fromStart = "<FROM>";
	char* fromEnd = "</FROM>";
	char* fromString = calloc(22,sizeof(char));
	strcat(fromString, fromStart);
	strcat(fromString,username);
	strcat(fromString,fromEnd);
	fromString[21]='\0';
	//printf("From is %s\n",fromString);
	char* encodeStart = "<ENCODE>";
	char* encodeEnd = "</ENCODE>";
	char* encodeString = calloc(26,sizeof(char));
	strcat(encodeString, encodeStart);
	strcat(encodeString,flag);
	strcat(encodeString,encodeEnd);
	encodeString[25]='\0';
	//printf("Encode is %s\n",encodeString);
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
	finalFrame[strlen(finalFrame)] = '\0';
	//printf("Writing %s to socket\n",finalFrame);
	int fdOut;
	sscanf(fdOut_One,"%d",&fdOut);									//extract fd to write to
	write(fdOut,finalFrame,strlen(finalFrame));						//write to pipe, be it fdOut or fdIn
	free(toString);
	free(fromString);
	free(encodeString);
	free(finalFrame);
	
	//printf("encoded string is %s of length %ld\n",encodedString,strlen(encodedString));
	
	//printf("Writing %s to socket in crc\n", encodedString);
	//write(fdOut,encodedString,1025);
	free(extendedData);
	free(rem);
	free(encodedString);
	encodedString = NULL;
	rem=NULL;
	extendedData=NULL;
}
