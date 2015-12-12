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
		contex.m_Fd = open(pFileName,O_RDONLY);
	}

	if(contex.m_Fd <= 0)
		return -1;
	
	bool isExit = false;
	pthread_t writeTid = -1;
	pthread_t readTid = -1;
	

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
	char buffer[4] = {0};
	while(!writeThredExit)
	{
		size_t leftSize = pContex->m_pRingBuffer->getBufferLeftSize();

		size_t realReadSize = (leftSize < 4)?leftSize:4;
		read(pContex->m_Fd,buffer,realReadSize);

		pContex->m_pRingBuffer->writeBuffer(buffer, realReadSize);
	}
	
	return 0;
}

void * read_entry(void *arg)
{
	Contex * pContex = (Contex *)arg;
	char buffer[4] = {0};
	while(!readThreadExit)
	{
		memset(buffer,0,4);
		pContex->m_pRingBuffer->readBuffer(buffer, 4);
		printf("buffer[%c][%c][%c][%c]\n",buffer[0],buffer[1],
						buffer[2],
						buffer[3]);
	}
	
	return 0;
}


