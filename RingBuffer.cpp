#include "RingBuffer.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#define RINGBUF_SIZE 2*1024*1024
#define RINGBUF_DEBUG(fmt,x...) printf("[%s][%d][DEBUG]"fmt,__FUNCTION__,__LINE__,##x)

#define RINGBUF_ERROR(fmt,x...) printf("[%s][%d][ERROR]"fmt,__FUNCTION__,__LINE__,##x)

#define RINGBUF_WARMING(fmt,x...) printf("[%s][%d][WARMING]"fmt,__FUNCTION__,__LINE__,##x)

#define RINGBUFFER_MIN(a,b)(a>b ? b:a)

RingBuffer::RingBuffer()
:m_pReadPtr(NULL),
m_pWritePtr(NULL),
m_pBuffer(NULL),
m_iBufferSize(0),
m_bBreakIO(false)
{
	pthread_cond_init(&m_iCondRead,NULL);
	pthread_mutex_init(&m_iMutxRead,NULL);

	pthread_cond_init(&m_iCondWrite,NULL);
	pthread_mutex_init(&m_iMutxWrite,NULL);
}

RingBuffer::~RingBuffer()
{
	if(m_pBuffer)
	{
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}
	m_pReadPtr = m_pWritePtr = NULL;
	m_iBufferSize = 0;

	pthread_mutex_destroy(&m_iMutxRead);
	pthread_mutex_destroy(&m_iMutxWrite);
	pthread_cond_destroy(&m_iCondRead);
	pthread_cond_destroy(&m_iCondWrite);
}

void RingBuffer::setBreakIO()
{
	m_bBreakIO = true;
}

void RingBuffer::initBuffer(int buffSize)
{
	m_iBufferSize = buffSize;
	m_pBuffer = new char[m_iBufferSize];
	m_pReadPtr = m_pWritePtr = m_pBuffer;
}

int RingBuffer::readBuffer(char *outBuf,int len)
{
	if(m_pBuffer == NULL)
	{
		RINGBUF_ERROR("Buffer no init\n");
	}

	pthread_mutex_lock(&m_iMutxRead);
	while(m_pWritePtr == m_pReadPtr)
	{		
		RINGBUF_WARMING("Buffer empty\n");
		struct timeval tv;
		struct timespec tvn; 

		gettimeofday(&tv,(struct timezone *) NULL);
		tvn.tv_sec = tv.tv_sec + 2;
		tvn.tv_nsec = (tv.tv_usec)*1000;
		
		pthread_cond_timedwait(&m_iCondRead,&m_iMutxRead,&tvn);
		if(m_bBreakIO)
		{
			RINGBUF_DEBUG("IO Broken\n");
			return 0;
		}
	}
	pthread_mutex_unlock(&m_iMutxRead);


	bool isSignalWrite = false;
	size_t bufLen = (m_pWritePtr > m_pReadPtr)?(m_pWritePtr - m_pReadPtr):
						(m_iBufferSize-(m_pReadPtr - m_pWritePtr));
	size_t outLen = (size_t)len;
	size_t readLen = RINGBUFFER_MIN(bufLen,outLen);

	if((m_pWritePtr+1-m_pBuffer)%m_iBufferSize == (m_pReadPtr-m_pBuffer)%m_iBufferSize)
	{
		isSignalWrite = true;
	}

	if(m_pWritePtr > m_pReadPtr)
	{
		pthread_mutex_lock(&m_iMutxWrite);
		
		memcpy(outBuf,m_pReadPtr,readLen);	
		m_pReadPtr = m_pReadPtr + readLen;
		if(isSignalWrite)
		{
			pthread_cond_signal(&m_iCondWrite);//signal write thread to write data to ringbuffer
		}
		pthread_mutex_unlock(&m_iMutxWrite);
	}else//Ringbuffer wrapround
	{
		if(m_iBufferSize-(m_pReadPtr - m_pBuffer) < readLen)
		{
			size_t readToEndLen = m_iBufferSize-(m_pReadPtr - m_pBuffer);
			pthread_mutex_lock(&m_iMutxWrite);
			memcpy(outBuf,m_pReadPtr,readToEndLen);
			m_pReadPtr = m_pBuffer;

			memcpy(outBuf+readToEndLen,m_pReadPtr,(readLen - readToEndLen));
			m_pReadPtr += (readLen - readToEndLen);

			if(isSignalWrite)
			{
				pthread_cond_signal(&m_iCondWrite);//signal write thread to write data to ringbuffer
			}
			pthread_mutex_unlock(&m_iMutxWrite);
		}else
		{
			pthread_mutex_lock(&m_iMutxWrite);
			memcpy(outBuf,m_pReadPtr,readLen);

			m_pReadPtr = m_pReadPtr+readLen;
			if(m_pReadPtr == (m_pBuffer + m_iBufferSize))
			{
				m_pReadPtr = m_pBuffer;
			}
			if(isSignalWrite)
			{
				pthread_cond_signal(&m_iCondWrite);//signal write thread to write data to ringbuffer
			}
			
			pthread_mutex_unlock(&m_iMutxWrite);
		}
	}

	return readLen;
}

int RingBuffer::writeBuffer(char *inBuf,int len)
{
	if(m_pBuffer == NULL)
	{
		RINGBUF_ERROR("Buffer no init\n");
		return -1;
	}

	pthread_mutex_lock(&m_iMutxWrite);
	while((m_pWritePtr+1-m_pBuffer)%m_iBufferSize == (m_pReadPtr-m_pBuffer)%m_iBufferSize)
	{
		RINGBUF_WARMING("Buffer full\n");
		struct timeval tv;
		struct timespec tvn; 

		gettimeofday(&tv,(struct timezone *) NULL);
		tvn.tv_sec = tv.tv_sec + 2;
		tvn.tv_nsec = (tv.tv_usec)*1000;
			
		pthread_cond_timedwait(&m_iCondWrite,&m_iMutxWrite,&tvn);
		//pthread_cond_wait(&m_iCondWrite,&m_iMutxWrite);
		if(m_bBreakIO)
		{
			pthread_mutex_unlock(&m_iMutxWrite);
			return -2;
		}
	}
	pthread_mutex_unlock(&m_iMutxWrite);

	size_t leftBufLen = (m_pWritePtr >= m_pReadPtr)?(m_iBufferSize - (m_pWritePtr-m_pReadPtr)-1):(m_pReadPtr - m_pWritePtr -1);

	size_t inBufLen = (size_t)len;
	size_t writeBufLen = RINGBUFFER_MIN(len, leftBufLen);

/*
	if(m_pWritePtr < m_pReadPtr)
	{
		memcpy(m_pWritePtr,inBuf,writeBufLen);

		pthread_mutex_lock(&m_iMutxRead);
		m_pWritePtr += writeBufLen;
		pthread_cond_signal(&m_iCondRead);
		pthread_mutex_unlock(&m_iMutxRead);
	}else
	{
*/		
		if((m_pWritePtr + writeBufLen) >= (m_pBuffer+m_iBufferSize))//write data to ringbuffer wrapround
		{
			size_t writeToEndLen = (m_pBuffer+m_iBufferSize) - m_pWritePtr;
			pthread_mutex_lock(&m_iMutxRead);
			memcpy(m_pWritePtr,inBuf,writeToEndLen);
			m_pWritePtr = m_pBuffer;

			if(writeBufLen-writeToEndLen > 0)
			{
				memcpy(m_pWritePtr,inBuf+writeToEndLen,writeBufLen-writeToEndLen);
				m_pWritePtr += (writeBufLen-writeToEndLen);
			}
			pthread_cond_signal(&m_iCondRead);
			pthread_mutex_unlock(&m_iMutxRead);
		}else
		{
			pthread_mutex_lock(&m_iMutxRead);
			memcpy(m_pWritePtr,inBuf,writeBufLen);
			m_pWritePtr += writeBufLen;
			pthread_cond_signal(&m_iCondRead);
			pthread_mutex_unlock(&m_iMutxRead);
		}
	//}
	return writeBufLen;
}

int RingBuffer::getBufferLeftSize()
{
	size_t leftBufLen = (m_pWritePtr >= m_pReadPtr)?(m_iBufferSize - (m_pWritePtr-m_pReadPtr)-1):(m_pReadPtr - m_pWritePtr -1);
	return leftBufLen;
}

