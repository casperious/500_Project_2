#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "encDec.h"

int main(int argc, char *argv[])
{
	consumer(argv[1],argv[2]);
	return 0;
}

/*

Consumer process. Reads from fdOut until producer closes write end.

Args:-
	fdOut_Zero - pipe file descriptor to read from
	fdIn_One - pipe file descriptor for children to write to
	
*/

int consumer(char* fdOut_Zero,char* fdIn_One)
{
	int fdOut;
	sscanf(fdOut_Zero,"%d",&fdOut);
	int fdIn;
	sscanf(fdIn_One, "%d", &fdIn);
	char buff[1025];
	ssize_t inp;
	while((inp=read(fdOut,buff,sizeof(buff)))>0)					//read from fdOut till producer closes. Store frame into buff
	{
		int pid1;
		pid1 = fork();
		if(pid1==0)
		{
			execl("deframe","deframe",buff,fdIn_One,NULL);			//call deframe for every frame read
		}
		else if (pid1>0)
		{
			wait(NULL);
		}
		else
		{
			printf("Failed fork");
		}
	}
	close(fdOut);													//close pipe read and write.
	close(fdIn);
	return 0;

}
