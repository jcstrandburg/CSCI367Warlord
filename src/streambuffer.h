/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

streambuffer.h

Specification for StreamBuffer module, which provides an interface between NetManager and Connection
=============================*/

#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include <string>

class StreamBuffer
{
	public:
		StreamBuffer();
		StreamBuffer( int sz);
		~StreamBuffer();
	void Write( const char *d, unsigned int l);//add the given data to the buffer, throwing an exception if the data cannot be accepted
	int Length() const;//the number of bytes of data in the buffer
	int Read( char *d, unsigned int l);//returns (and consumes) the amount of data actually copied, if d is null the data will just be consumed
unsigned int Window() const;//returns the maximum number of bites that the buffer can currently accept
	void Clear();//erases everything from the buffer
const char& operator[] ( unsigned int index);

//debugging junk, remove this later
	char* tostring();

	//private:
	char *data;
unsigned int cap;
unsigned int len;
	
		//make it non-copyable
		StreamBuffer( StreamBuffer&);
		StreamBuffer& operator = (StreamBuffer&);
		
};

#endif
