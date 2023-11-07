#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char* argv[])
{
	decode(argv);
}

/*

Decode the 7 character blocks into their ASCII values and store to output string. If fd passed is -1, write to data.done. Otherwise send decoded data to toUpper

Args:-
	blocks - string array with 7 character binary encoded blocks of characters to be decoded

*/

int decode(char** blocks)
{
	//printf("In decoder\n");
	int len = 0;
	while(blocks[len+3]!=NULL)													//get number of characters to decode
	{
		len++;
	}
	int numChars = len;
	//printf("Numchars in decode is %d\n",numChars);
	char* fdIn_One = blocks[1];													//get pipe fd to write to
	char* file = blocks[2];
	//printf("File being saved to is %s\n",file);
	FILE* ptr;
	if(strcmp(file,"\n")==0)
	{
		//Do nothing
	}
	else
	{
		
		ptr = fopen(file,"a");
	}
	char* data = malloc(numChars+1);											//initialize outData
	for(int i =0;i<=numChars;i++)
	{
		data[i]='\0';
	}
	int k =0;
	char c;
	for(int i =3;i<=numChars+1;i++)												//loop through number of characters 
	{
		c= (int)strtol(blocks[i],NULL,2);										//convert binary string to int ASCII value
		data[k] = c;															//save to outData
		k++;
	}
	data[numChars] = '\0';														//last character is NULL, terminating string
	char charCount[64];
	sprintf(charCount,"%d",numChars+1);											//store number of characters as string to pass as arg
	int fd;
	sscanf(fdIn_One,"%d",&fd);													//store pipe fd as int
	printf("%s\n",data);
	
	if(strcmp(file,"\n")==0)
	{
	
	}
	else
	{
		fputs(data,ptr);														//write decoded string to data.done (Capitalized)
		fclose(ptr);
	}
	free(data);																//free space by malloc
	data=NULL;
	return 1;
}
