#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char *argv[])
{
	encode(argv[1],argv[2],argv[3],argv[4],argv[5]);
	return 0;
}

/*

Converts length and <=64 character frame each into 7 character long binary encoded data

Args:-
	inData = string to be encoded
	len = length of string to be encoded
	fdOut_One = file descriptor for pipe to write to. Different if producer is calling, or if consumer's child is calling it.
	isCap = flag to check if consumer child is calling, or if producer is.
	flag = flag for hamming or crc
*/
int encode(char *inData,char* len, char* fdOut_One, char* isCap,char* flag){
	char bin[1025]="";												//binary encoding of length + characters
	int length;
	sscanf(len,"%d",&length);										//storing len as an int
	int num = length;
	int j =6;
	char res[7]="0000000";
	while(num>0)													//converting length of string into its binary encoded 7 character string
	{
		int mod = num%2;
		res[j]=mod+'0';
		num=num/2;
		j--;
	}
	strncat(bin,res,7);
	
	for(int i =0;i<length;i++){										//loop through all characters in inData
		int ascii = inData[i];										//Get ascii int value of character
		int num = ascii;
		int j = 6;
		char result[7]="0000000";
		while(num>0)												//get binary representation of ascii value and store it in result
		{
			int mod = num%2;										//get remainder by dividing ascii by 2
			result[j] = mod+'0';									//store the character value of 0 or 1 into result starting from the highest bit
			num=num/2;												//divide ascii by 2
			j--;													//move to next lowest bit to be stored	
		}
		strncat(bin,result,7);
	}
	int pid;
	pid = fork();
	if(pid==0){
		execl("parityAddService","parityAddService",bin,fdOut_One,isCap,flag,NULL);			//call parityAddService with binary encoded string, fd to write to, isCap
	}
	else if(pid>0)
	{
		wait(NULL);
	}
	else
	{
		printf("No fork\n");
	}
	
	return 0;

}
