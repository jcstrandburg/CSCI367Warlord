/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

wlclient.h

Spec for the WLClient module, which represents clients in the server application
=============================*/

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "wlconnection.h"

class NetMessage;

class WLClient
{
public:
enum ConnectionStatus
{
	Uncertified,
	InLobby,
	InGame,
	Disconnected,
};
typedef std::string Identifier;
	
	WLClient( WLConnection &c);
	~WLClient();
ConnectionStatus GetStatus() const;
Identifier GetIdentifier() const;
void	EnterLobby( const Identifier &id);
void	EnterGame();
void	ReturnToLobby();
unsigned int GetStrikes();
void	TakeStrike();
void	Disconnect();
void	IncreaseChatThrottle();
void	DecreaseChatThrottle();
unsigned GetChatThrottle();

WLConnection connection;

private:
Identifier ident;
ConnectionStatus status;
unsigned int strikes;
unsigned int chatThrottle;
};


#endif /*CLIENT_H*/
