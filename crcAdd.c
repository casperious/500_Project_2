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
	char* retString = calloc(34,sizeof(char));													//hardcoded length of divisor
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
	while(i<len)						//i = 534
	{
		//printf("Extended Data is %s\n",extendedData);
		if(extendedData[i]=='0')
		{
			i++;
		}
		else
		{
			/*
			printf("\n");
			for(int j =0;j<33;j++)
			{
				printf("%c",extendedData[i+j]);
			}
			printf("\n");
			printf("%s \n --------------------------------------------------------------------------\n",crc_gen);
			*/
			char* xor_string = calloc(outLen+1,sizeof(char));
			char* x = calloc(strlen(crc_gen)+1,sizeof(char));
			for(int j = 0;j<strlen(crc_gen);j++)
			{
				x[j] = extendedData[i+j];
			}
			x[33]='\0';
			for(int j =0;j<i;j++)
			{
				xor_string[j]='0';
			}
			//xor_string[outLen] = '\0';
			char* retstring = XOR(x,crc_gen);
			//printf("%s\n================================================================\n",retstring);
			free(x);
			x=NULL;
			for(int j =0;j<33;j++)
			{
				xor_string[i+j] = retstring[j];
			}
			free(retstring);
			retstring = NULL;
			for(int j = i+33;j<strlen(extendedData);j++)
			{
				xor_string[j] = extendedData[j];
			}
			xor_string[outLen]='\0';
			//printf("XOR_String is %s\n",xor_string);
			for(int j =0;j<strlen(extendedData);j++)
			{
				extendedData[j] = xor_string[j];
			}
			//printf("%s\n=================================================================\n",extendedData);
			free(xor_string);
			xor_string = NULL;
			i++;
		}
	}
	char* rem = malloc(strlen(crc_gen));
	//printf("extracting remainder from %s\n",extendedData);
	int j =0;
	for(int i =strlen(inData);i<outLen;i++)
	{
		rem[j]=extendedData[i];
		j++;
	}
	rem[32]='\0';
	
	//printf("Remainder is %s of length %ld\n",rem,strlen(rem));					
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
	//printf("encoded string is %s of length %ld\n",encodedString,strlen(encodedString));
	int currPid = getpid();
	srand(currPid);												//use current pid as seed
	int random = rand();
	int idx = random%outLen;									//get index to be flipped	
	if(isCap[0]=='1')
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
		//printf("encoded string is %s of length %ld\n",encodedString,strlen(encodedString));
	}
	encodedString[outLen]='\0';
	//printf("encoded string is %s of length %ld\n",encodedString,strlen(encodedString));
	/*FILE* ptr;
	ptr = fopen("check.inpf","a");
	fputs(encodedString,ptr);
	fclose(ptr);
	*/
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
	free(extendedData);
	free(rem);
	free(encodedString);
	//free(finalFrame);
	

	
	//printf("Writing %s to socket in crc\n", encodedString);
	//write(fdOut,encodedString,1025);
	
	encodedString = NULL;
	rem=NULL;
	extendedData=NULL;
}
