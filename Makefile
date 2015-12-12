INCLUDE = -I ./include
LIB = -lpthread


ringbuffer : main.o RingBuffer.o
	g++ $(LIB) -o ringbuffer main.o RingBuffer.o

RingBuffer.o : RingBuffer.cpp include/RingBuffer.h
	g++ $(INCLUDE) -c RingBuffer.cpp

main.o : main.cpp include/RingBuffer.h
	g++ $(INCLUDE) -c main.cpp

clean:
	rm ringbuffer;rm -rf *.o
