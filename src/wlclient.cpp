/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

wlclient.cpp

Implementation for the WLClient module, which represents clients in the server application
=============================*/

#include <string>
#include <assert.h>

#include "wlclient.h"
#include "netmessage.h"
#include "wlconnection.h"

using namespace std;

WLClient::WLClient( WLConnection &c):
	ident("********"), status( WLClient::Uncertified), strikes(0), chatThrottle( 0)
{
	connection = c;
}

WLClient::~WLClient()
{
	
}
	
WLClient::ConnectionStatus WLClient::GetStatus() const
{
	return status;
}

WLClient::Identifier WLClient::GetIdentifier() const
{
	return ident;
}

void	WLClient::EnterLobby( const WLClient::Identifier &id)
{
	assert( status == Uncertified);
	status = InLobby;
	ident = id;
}

void	WLClient::ReturnToLobby()
{
	status = InLobby;
}

void	WLClient::EnterGame()
{
	status = InGame;
}

unsigned int WLClient::GetStrikes()
{
	return strikes;
}

void	WLClient::TakeStrike()
{
	strikes ++;
}

void	WLClient::Disconnect()
{
	status = Disconnected;
}

void	WLClient::IncreaseChatThrottle()
{
	chatThrottle++;
}

void	WLClient::DecreaseChatThrottle()
{
	if ( chatThrottle > 0 )
		chatThrottle--;
}

unsigned WLClient::GetChatThrottle()
{
	return chatThrottle;
}
