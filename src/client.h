/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

client.h

Specification for Client module, which provides the foundation of the functionality of the client application
=============================*/

#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include "interface.h"
#include "card.h"
#include "netmanager.h"
#include "clientplaystate.h"
#include "timer.h"

typedef set<string> PlayerList;


//Spec for the private implementation of the Client object
class Client
{
public:
enum Task
{
	NoTask,
	AutoPlay,
	GetPlay,
	GetSwap,
};
	
	
	Client( InterfacePtr i, bool doAutoMode, bool doLogging);
	~Client();
	void	Connect( const string &host, int port);
	void	ConnectAs( const string &host, int port, const string &name);
	bool 	ExitCondition();
	void	Update();

private:
	
bool	AttemptPlay( CardList &c);
void 	TransmitMessage( WLConnection &c, NetMessage &msg);
void 	ProcessMessage( const NetMessage &msg);
void	ProcessLobbyMessage( const NetMessage &msg);
void	ProcessHandMessage( const NetMessage &msg);
void	ProcessTableMessage( const NetMessage &msg);
void	ProcessStrikeMessage( const NetMessage &msg);
void	UnpackTableStatus( const NetMessage &msg, ClientPlayState &s);
void	AutoSelectCards( CardList &dst);
void	LocalTurnActivation();
void	SwapActivation();

private:
NetManager net;
WLConnection conn;
bool exitCondition;
ClientPlayState state;
InterfacePtr iface;
PlayerList lobby;
Task task;
bool invalidPlay;//ugly little flag we need for message processing
Card swapCard;
bool autoMode;
Timer autoTimer;
bool lostConnection;
};


#endif /*CLIENT_H_*/
