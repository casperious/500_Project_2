#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"
#include <math.h>
int main(int argc, char *argv[])
{
	addCRC(argv[1],argv[2],argv[3]);
	return 0;
}

void addCRC(char* inData, char* fdOut_One, char* isCap)
{
	char* crc_gen = "10000010011000001000111011011011"; //x32+ x26+ x23+ x22+ x16+ x12+ x11+ x10+ x8+ x7+ x5+ x4+ x2+ x + 1
	int len = strlen(inData);
	printf("length of generator is %ld and length of indata is %ld\n",strlen(crc_gen),strlen(inData));
	int outLen = len+32-1;
	printf("%d\n",outLen);
	char* extendedData = calloc(outLen,sizeof(char));
	printf("%ld\n",strlen(extendedData));
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
		printf("%c",extendedData[i]);
	}
	
	printf("\n-----------------------------------------\n");
	printf("inData with padded 0's is of length %ld is %s\n",strlen(extendedData),extendedData);
	
	free(extendedData);
	
}
