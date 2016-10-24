/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

wlconnection.cpp

Specification for WLConection module, which sits on top of the Connection class and
provides some message parsing and transmittal capability
=============================*/

#ifndef WLCONNECTION_H
#define WLCONNECTION_H

#include "connection.h"

class NetMessage;

class WLConnection: public Connection
{
public:
enum MessageStatus
{
	NoMessage,
	LengthExceeded,
	Malformed,
	GoodMessage,
};

	WLConnection();
	~WLConnection();
MessageStatus GetMessageStatus();
void	ClearGarbage();//removes all unnecessary data from the buffer (malformed messages or otherwise unusable data) and resets the status if there was an error
void 	GetMessage( NetMessage& m);
void	SendMessage( NetMessage& m);
void	SetMaxMessageLength( unsigned int l);
WLConnection& operator=( const Connection &c);

private:
MessageStatus status;
int	malformityIndex;//the location of the furthest malformity we have found in the buffer (-1 if not relevant)
int	messageIndex;//the index where the message starts (-1 if not relevant)
int messageLen;
int	msgLength;//-1 if no message is available	
int maxLen;
};

#endif
