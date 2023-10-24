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
	checkCRC(argv[1],argv[2],argv[3]);
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

void checkCRC(char* inData, char* fdOut_One, char* isCap)
{
	int i =0;
	char*  check = malloc(strlen(inData));
	for(int k=0;k<strlen(inData);k++)
	{
		check[k] = inData[k];
	}
	check[strlen(inData)]='\0';	
	printf("check is %s with len %ld\n",check,strlen(check));
	while(i+strlen(crc_gen)<strlen(inData))
	{
		if(check[i]=='0')
		{
			i++;
		}
		else
		{
			printf("%s\n",check);
			for(int j = 0;j<i;j++)
			{
				printf(" ");
			}
			printf("%s\n",crc_gen);
			char* xor_string = calloc(strlen(inData),sizeof(char));
			char* x = calloc(strlen(crc_gen),sizeof(char));
			for(int j = i;j<strlen(crc_gen)+i;j++)
			{
				x[j-i] = check[j];
			}
			for(int j =0;j<i;j++)
			{
				xor_string[j]='0';
			}
			char* retstring = XOR(x,crc_gen);
			free(x);
			x=NULL;
			for(int j =i;j<i+strlen(crc_gen);j++)
			{
				xor_string[j] = retstring[j-i];
			}
			free(retstring);
			retstring = NULL;
			for(int j = i+strlen(crc_gen);j<strlen(inData);j++)
			{
				xor_string[j] = check[j];
			}
			//printf("XOR_String is %s\n",xor_string);
			for(int j =0;j<strlen(inData);j++)
			{
				check[j] = xor_string[j];
			}
			free(xor_string);
			xor_string = NULL;
			i++;
		}
	}
	
	printf("Remainder is %s\n",check);
	for(int j =0;j<strlen(check);j++)
	{
		if(check[j]=='1')
		{
			printf("Data received has been corrupted during transmission\n");
			break;
		}
	}
	free(check);
	
	check=NULL;
	


}
