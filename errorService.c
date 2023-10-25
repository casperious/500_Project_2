#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char* argv[])
{
	error(argv[1],argv[2],argv[3],argv[4],argv[5]);
	return 0;
}

/*

Error generator. Uses current pid as seed for srand. Generates random number, and does %length to get index to flip

Args:-
	inData = string to be encoded
	len = length of string to be encoded
	fdOut_One = file descriptor of pipe to write to
	isCap = flag for if consumer called, or producer
	flag = flag for if hamming or crc
*/


int error(char *inData,char* len, char* fdOut_One,char* isCap,char* flag){

	char bin[1025]="";											//binary encoding of length + characters
	int length;
	sscanf(len,"%d",&length);									//store length as int
	int num = length;
	int j =6;
	char res[7]="0000000";
	while(num>0)												//get 7 character long binary encoding of length
	{
		int mod = num%2;
		res[j]=mod+'0';
		num=num/2;
		j--;
	}
	strncat(bin,res,7);
	int currPid = getpid();
	srand(currPid);												//use current pid as seed
	int random = rand();
	int idx = random%length;									//get index to be flipped	
	printf("Index to be flipped is %d and char is %c\n",idx, inData[idx/7]);
	for(int i =0;i<length;i++){
		int ascii = inData[i];										//Get ascii int value of character
		int num = ascii;
		int j = 6;
		char result[7]="0000000";
		while(num>0)												//get binary representation of ascii value and store it in 		result
		{
			int mod = num%2;										//get remainder by dividing ascii by 2
			result[j] = mod+'0';									//store the character value of 0 or 1 into result starting from the highest bit
			num=num/2;												//divide ascii by 2
			j--;													//move to next lowest bit to be stored	
		}
		strncat(bin,result,7);
	}
	if(bin[idx+7]=='1')												//if bit is 1, flip to 0
	{
		bin[idx+7]='0';
	}	
	else															//else flip bit to 1
	{
		bin[idx+7]='1';
	}
	int pid;
	pid = fork();
	if(pid==0){
		execl("parityAddService","parityAddService",bin,fdOut_One,isCap,flag,NULL);		//pass erroneous string to parityAdd with pipe fd and isCap
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
