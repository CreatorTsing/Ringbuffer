#include <pthread.h>
#include <sys/types.h>
#include "RingBuffer.h"
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

void * write_entry(void *arg);
void * read_entry(void *arg);
typedef struct Contex
{
	int m_Fd;
	RingBuffer * m_pRingBuffer;
}Contex;

bool writeThredExit = false;
bool readThreadExit = false;
int main(int argc,char** argv)
{
	char c;
	char *pFileName = NULL;
	Contex contex;
	memset(&contex,0,sizeof(Contex));
	if(argc > 1)
	{
		pFileName = argv[1];
		printf("%s\n",pFileName);
		contex.m_Fd = open(pFileName,O_RDONLY);
	}
	else
	{
		printf("warning,pls enter a file name\n");
		return -1;
	}
	if(contex.m_Fd <= 0)
		return -1;
	
	bool isExit = false;
	pthread_t writeTid;
	pthread_t readTid;
	

	contex.m_pRingBuffer = new RingBuffer();
	contex.m_pRingBuffer->initBuffer(32);

	pthread_create(&writeTid,NULL,write_entry,(void*)&contex);
	pthread_create(&readTid,NULL,read_entry,(void*)&contex);
	
	while(c = getchar())
	{
		switch(c)
		{
		case 'q':
			isExit = true;
			break;
		default:
			break;
		}

		if(isExit)
			break;
	}

	contex.m_pRingBuffer->setBreakIO();
	writeThredExit = true;
	readThreadExit = true;

	void * reval = NULL;
	pthread_join(writeTid,&reval);
	pthread_join(readTid,&reval);
	
	return 0;
}

void * write_entry(void *arg)
{
	Contex * pContex = (Contex *)arg;
	char buffer[5] = {0};
	while(!writeThredExit)
	{
		int writeLen = 0;
		size_t readLen = read(pContex->m_Fd,buffer,4);
		if(readLen > 0)
		{
			do
			{
				writeLen = pContex->m_pRingBuffer->writeBuffer(buffer+writeLen, readLen);
				if(writeLen < 0)
				{
					printf("writeLen is %d\n",(int)writeLen);
					goto exit_thread;
				}
				readLen = readLen - writeLen;
			}while(readLen > 0);
		}
		else
		{
			//printf("readLen is %d\n",(int)readLen);
			sleep(1);
		}
		sleep(1);
	}
exit_thread:
	printf("write_entry exit\n");
	return 0;
}

void * read_entry(void *arg)
{
	Contex * pContex = (Contex *)arg;
	char buffer[5] = {0};
	while(!readThreadExit)
	{
		memset(buffer,0,5);
		pContex->m_pRingBuffer->readBuffer(buffer, 4);
		printf("buffer[%s]\n",buffer);
	}
	
	return 0;
}


