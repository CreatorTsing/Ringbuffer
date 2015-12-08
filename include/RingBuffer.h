

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

	size_t readBuffer(char *outBuf,int len,bool* breakCond);
	size_t writeBuffer(char *inBuf,int len,bool* breakCond);
	void setBreakIO();
private:
	pthread_cond_t m_iCondRead;
	pthread_mutex_t	m_iMutxRead;

	pthread_cond_t m_iCondWrite;
	pthread_mutex_t m_iMutxWrite;
}


