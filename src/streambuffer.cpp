/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

streambuffer.cpp

Implementation for StreamBuffer module, which provides an interface between NetManager and Connection
=============================*/

#include <stdio.h>
#include <cstring>

#include "streambuffer.h"

StreamBuffer::StreamBuffer()
{
	StreamBuffer( 0);
}

StreamBuffer::StreamBuffer( int sz):
cap( sz), len(0)
{
	if ( sz != 0)
		data = new char[sz];
	else
		data = 0;
}

StreamBuffer::~StreamBuffer()
{
	if ( data != 0)
	{
		delete[] data;
		data = 0;
	}
}

void StreamBuffer::Write( const char *d, unsigned int l)
{
	if ( l > Window() )
	{
		throw "StreamBuffer.Accept passed data larger than window";
	}
	if ( l < 0)
	{
		throw "StreamBuffer.Accept passed an insane value for l";
	}	
	
	memmove( &data[len], d, l);
	len += l;	
}

int StreamBuffer::Length() const
{
	return len;
}

int StreamBuffer::Read( char *d, unsigned int l)//returns the amount of data actually copied
{
	if ( len <= 0 ) return 0;

	if ( l > len)
		l = len;
	
	if ( d != 0 )
		memmove( d, data, l);//copy the requested data to d
	memmove(data, &data[l], cap-l);//shift the remaining data left
	len -= l;

	return l;
}

unsigned int StreamBuffer::Window() const
{
	return cap-len;
}

void StreamBuffer::Clear()
{
	Read( 0, len);
}

const char& StreamBuffer::operator[] ( unsigned int index)
{
	if ( index > len )
		throw "Invalid index in Streambuffer::operator[]";

	return data[index];
}

char* StreamBuffer::tostring()
{
static char buffer[10000];

	memcpy( buffer, data, len);
	buffer[len] = 0;
	return buffer;
}
