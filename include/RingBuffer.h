#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <pthread.h>
#include <sys/types.h>

class RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();
	
	char * m_pReadPtr;
	char * m_pWritePtr;
	char * m_pBuffer;
	size_t m_iBufferSize;
	
	bool m_bBreakIO;
	void initBuffer(int buffSize);

	void setBreakIO();

	int readBuffer(char *outBuf,int len);
	int writeBuffer(char *inBuf,int len);
	int getBufferLeftSize();
	
private:
	pthread_cond_t m_iCondRead;
	pthread_mutex_t	m_iMutxRead;

	pthread_cond_t m_iCondWrite;
	pthread_mutex_t m_iMutxWrite;
};

#endif
