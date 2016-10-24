/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

connection.h

Specification for Connection module, which provide an oop model for managing tcp socket connections
=============================*/

#ifndef CONNECTION_H
#define CONNECTION_H

#include "streambuffer.h"
#include <tr1/memory>


class NetManager;
class StreamBuffer;
class NetMessage;

class ConnectionBuffer
{
	public:
		ConnectionBuffer( int inSize, int outSize);
bool	IsOpen();
void	Close();
StreamBuffer in, out;
	private:
bool	open;
};

class Connection 
{
	public:
		Connection();
		Connection( std::tr1::shared_ptr<ConnectionBuffer> b);
		~Connection();
	bool IsOpen() const;
	void Close();
	int	Get( char *dat, int l);//attempt get l bytes of raw data from the buffer, returning the actual length gotten. If dat is null the data will just be consumed
	void TransmitRawData( const char *data, int len);
	
	//protected:
std::tr1::shared_ptr<ConnectionBuffer> buffer;
};

#endif
