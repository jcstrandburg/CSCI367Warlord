/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

connection.cpp

Implementation for Connection module, which provide an oop model for managing tcp socket connections
=============================*/

#include <stdio.h>

#include "connection.h"
#include "netmanager.h"
#include "streambuffer.h"

ConnectionBuffer::ConnectionBuffer( int inSize, int outSize):
	in( inSize), out(outSize), open( true)
{
}

bool ConnectionBuffer::IsOpen()
{
	return open;
}

void ConnectionBuffer::Close()
{
	open = false;
}


Connection::Connection()
{
}

Connection::Connection( std::tr1::shared_ptr<ConnectionBuffer> b):
buffer(b)
{
}

Connection::~Connection() 
{
}

bool Connection::IsOpen() const
{
	return (buffer && buffer->IsOpen());
}

void Connection::Close()
{
	if ( !IsOpen() ) return;

	buffer->Close();
	buffer.reset();
}

int Connection::Get( char *dat, int l)
{
	return buffer->in.Read( dat, l);
}

void Connection::TransmitRawData( const char *data, int len)
{
	buffer->out.Write( data, len);
}
