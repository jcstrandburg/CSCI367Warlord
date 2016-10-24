/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

wlconnection.cpp

Implementation for WLConection module, which sits on top of the Connection class
and provides some message parsing and transmittal capability
=============================*/

#include <stdio.h>
#include <assert.h>

#include "wlconnection.h"
#include "netmessage.h"

WLConnection::WLConnection():
status( NoMessage), malformityIndex( -1), messageIndex( -1), msgLength( -1), maxLen( 100)
{
}


WLConnection::~WLConnection()
{
}


WLConnection::MessageStatus WLConnection::GetMessageStatus()
{
int i, j, len;

	if ( buffer == 0)
		return status = NoMessage;

	//check to see if the status needs to be updated
	if ( status != NoMessage )
	{
		return status;
	}

	len = buffer->in.Length();
	for ( i = 0; i < len; i++)
	{
		if ( buffer->in[i] == '[' )
		{
			for ( j = i+1; j < len; j++)
			{
				if ( j - i >= maxLen)
				{
					malformityIndex = j-1;
					return status = LengthExceeded;
				}

				switch ( buffer->in[j])
				{
					case '[':
						malformityIndex = j;
						return status = Malformed;
						break;
					case ']':
						messageIndex = i;
						messageLen = j-i+1;
						return status = GoodMessage;
						break;
				}
			}
			//if we got to the end of the buffer and nothing notable happened then we're still waiting for input
			return status = NoMessage;
		}
	}

	//if we got here then we searched the whole buffer and found no open message character,
	//lets just clear it because there is nothing useful
	buffer->in.Clear();
	return status = NoMessage;
}

//removes all unnecessary data from the buffer (malformed messages or otherwise unusable data)
void	WLConnection::ClearGarbage()
{
int i;

	assert( buffer);

	//if malformityIndex == -1, then we start at 0, otherwise start at the first character that may be good
	//iterate until we reach the end of the buffer (thus i = buffer length) or we reach an open character
	for ( i = malformityIndex + 1; i < buffer->in.Length(); i++)
	{
		if ( buffer->in[i] == '[' )
		{
			break;
		}
	}

	//consume the first i bytes of the buffer, update the messageIndex if it was pointing to a message
	buffer->in.Read( 0, i);
	malformityIndex = -1;
	
	if ( messageIndex >= 0 )
	{
		messageIndex -= i;
	}
	
	//reset the status, as we ahve modified the buffer etc
	status = NoMessage;
}


void 	WLConnection::GetMessage( NetMessage& m)
{
	if( !buffer)
		return;
	
	if ( status != GoodMessage )
	{
		m.Reset( NetMessage::Invalid);
	}
	else
	{
		char buff[messageLen+1];
		ClearGarbage();
		buffer->in.Read( buff, messageLen);
		buff[ messageLen] = '\0';
		messageLen = messageIndex = -1;
		status = NoMessage;
		//printf( "(%s)\n", buff);
		m.Parse( buff);
	}
}

void 	WLConnection::SendMessage( NetMessage& m)
{
	if( buffer)
	{
	string s = m.Serialize();	
	
		if ( buffer->out.Window() < s.size())
			throw "WLConnection::SendMessage attempting to send too much data through Streambuffer::Accept";
		buffer->out.Write( s.c_str(), s.size());
	}
}

void 	WLConnection::SetMaxMessageLength( unsigned int l)
{
	maxLen = l;
}

WLConnection& WLConnection::operator=( const Connection &c)
{
	Connection::operator=(c);
	return *this;
}
