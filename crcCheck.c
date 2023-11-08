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
//10000010011000001000111011011011100000000000000000000000000000000
//
void checkCRC(char* inData, char* fdIn_One, char* flag,char* file)
{
	inData[568]='\0';
	int i =0;
	//printf("InData in checkCRC is %s of length %ld\n",inData,strlen(inData));
	//printf("length of crcGen is %ld\n",strlen(crc_gen));
	char*  check = calloc(strlen(inData),sizeof(char));
	for(int k=0;k<strlen(inData);k++)
	{
		check[k] = inData[k];
	}
	check[568]='\0';	
	//printf("check is %s with len %ld\n",check,strlen(check));
	//printf("Comparing inData and check %d\n",strcmp(inData,check));
	/*char* xor_string = calloc(strlen(inData),sizeof(char));
	char* x = calloc(strlen(crc_gen),sizeof(char));
	while(i+33<strlen(inData))				//+strlen(crc_gen)
	{
		if(check[i]=='0')
		{
			//xor_string[i]='0';
			i++;
			
		}
		else
		{
			
			//printf("%s\n",check);
			//for(int j = 0;j<i;j++)
			//{
			//	printf(" ");
			//}
			//printf("%s\n",crc_gen);
			
			for(int j = i;j<strlen(crc_gen)+i;j++)		//maybe -1
			{
				x[j-i] = check[j];
			}
			x[strlen(x)]='\0';
			//for(int j =0;j<i;j++)
			//{
			//	xor_string[j]='0';
			//}
			xor_string[strlen(xor_string)]='\0';
			char* retstring = XOR(x,crc_gen);
			
			for(int j =i;j<i+strlen(crc_gen);j++)
			{
				xor_string[j] = retstring[j-i];
			}
			//free(retstring);
			//retstring = NULL;
			for(int j = i+strlen(crc_gen);j<strlen(inData);j++)
			{
				xor_string[j] = check[j];
			}
			xor_string[strlen(inData)]='\0';
			//printf("XOR_String is %s\n",xor_string);
			for(int j =0;j<strlen(inData);j++)
			{
				check[j] = xor_string[j];
			}
			check[strlen(inData)]='\0';
			//printf("last char in check is %c\n",check[strlen(inData)]);
			
			i++;
		}
	}*/
	while(i+32<strlen(inData))						//i = 534
	{
		//printf("Extended Data is %s\n",extendedData);
		if(check[i]=='0')
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
			/*printf("\n");
			for(int j =0;j<33;j++)
			{
				printf("%c",check[i+j]);
			}
			printf("\n");
			printf("%s \n --------------------------------------------------------------------------\n",crc_gen);
			*/
			char* xor_string = calloc(strlen(inData)+1,sizeof(char));
			char* x = calloc(strlen(crc_gen)+1,sizeof(char));
			for(int j = 0;j<strlen(crc_gen);j++)
			{
				x[j] = check[i+j];
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
			for(int j = i+33;j<strlen(check);j++)
			{
				xor_string[j] = check[j];
			}
			xor_string[strlen(inData)]='\0';
			//printf("XOR_String is %s\n",xor_string);
			for(int j =0;j<strlen(check);j++)
			{
				check[j] = xor_string[j];
			}
			//printf("%s\n=================================================================\n",check);
			free(xor_string);
			xor_string = NULL;
			i++;
		}
	}
	//printf("Remainder is %s\n",check);												//01001001000111011110011101100010
	
	//free(x);
	//x=NULL;
	//free(xor_string);
	//xor_string = NULL;
	int j =0;
	//for(int j =0;j<strlen(check);j++)
	while(j<strlen(check))
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
		//printf("correct?\n");
	}
	free(check);
	char* send = calloc(strlen(inData)-strlen(crc_gen)+2,sizeof(char));
	//strncpy(send,inData,strlen(inData)-strlen(crc_gen)+1);
	for(int i =0;i<strlen(inData)-strlen(crc_gen)+1;i++)
	{
		send[i]=inData[i];
	}
	send[strlen(send)]='\0';
	//printf("sending %s of length %ld\n",send, strlen(send));
	check=NULL;
	int pid;
	pid = fork();
	if(pid==0)
	{
		execl("deframe","deframe",send,fdIn_One,flag,file,NULL);
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
