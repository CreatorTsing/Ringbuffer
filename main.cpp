#include <pthread.h>

void * write_entry(void *arg);
void * read_entry(void *arg);
typedef struct Contex
{
	int m_Fd;
	RingBuffer * m_pRingBuffer;
}Contex;

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
	
	bool writeThredExit = false;
	bool readThreadExit = false;
	

	contex->pRingBuffer = new pRingBuffer();
	contex->pRingBuffer->initBuffer(32);

	pthread_create(&writeTid,NULL,write_entry,(void*)&contex);
	pthread_create(&readTid,NULL,read_entry,(void*)&contex);
	
	while(c = getchar())
	{
		switch()
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

	pthread_join();
	pthread_join();
	
	return 0;
}

void * write_entry(void *arg)
{
	RingBuffer * pRingBuf = (RingBuffer *)arg;
	while(1)
	{
		
	}
	
	return 0;
}

void * read_entry(void *arg)
{
	
}


