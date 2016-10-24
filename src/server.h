/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

server.h

Specification for the server class, which does all the logic for the server application
=============================*/

#ifndef SERVER_H
#define SERVER_H

#include <tr1/memory>

#include "warlord.h"
#include "wlclient.h"
#include "interface.h"
#include "netmanager.h"
#include "serverplaystate.h"
#include "timer.h"

class Server;
class PlayerState;
class WLConnection;
class NetMessage;

typedef tr1::shared_ptr<WLClient> WLClientPtr;
typedef list<WLClientPtr> ClientContainer;
typedef ClientContainer::iterator ClientIterator;

class Server
{
public:

	Server( InterfacePtr ip, int prt, int tout, int lobbTime, int minPlr, bool log);
	~Server();
void 	EchoTableStatus();
bool 	ExitCondition();
void 	StartUp();
void	Update();

private:
//basic updating functions
void 	ProcessMessages();
void	CheckForConnections();
void	CheckForDisconnects();
void	DeleteDisconnectedClients();
void	DecrementChatThrottles();

//connection handling methods
bool 	ClientNameConnected( string &s);
void 	GiveStrike( WLClientPtr c, Errors::ErrorCode err);
void	HandleInGameDisconnect( PlayerState *p);
string 	Mangle( string &s);
string 	PreMangle( string &s);
void	RejectConnection( WLConnection c);
string 	ValidateName( string &s);

//game state methods
void 	AdvancePlayerIndex();
void	CancelSwap();
bool	CheckGameOver();
void	DealHands( vector<CardList> &hands);
void	DoCardSwap( Card giveCard, Card takeCard);//take takeCard from scumbag and give it to warlord
void	DoPlay( PlayerState *p, const CardList &cards);
void	DoPreGame();
bool	InGame();
void 	SeatClient( WLClientPtr c);
void 	StartGame();
bool	ValidPlay( PlayerState *p, const CardList &cards);

//message creation creation and transmittal methods
void	BroadcastMessage( NetMessage &msg);
void	SendChat( const string &name, const std::string &s);
void	SendHandMsg( PlayerState *p);
void	SendTableMsg( PlayerState *p);

//message type specific handler functions
void	HandleJoinMessage( WLClientPtr c, NetMessage &msg);
void	HandleChatMessage( WLClientPtr c, NetMessage &msg);
void 	HandlePlayMessage( WLClientPtr c, NetMessage &msg);
void	HandleSwapMessage( WLClientPtr c, NetMessage &msg);
void	HandleCHandMessage( WLClientPtr c, NetMessage &msg);

//status message builders
void	BuildLobbyStatusMsg( NetMessage &msg);
void	BuildTableStatusMsg( NetMessage &msg);
void	BuildHandMsg( NetMessage &msg, const CardList &c);

InterfacePtr iface;
	NetManager net;
	ClientContainer clients;
	PlayState state;
	Timer chatThrottleTimer;

	bool exitCondition;
	int listenPort;
unsigned int playTimeout;
unsigned int lobbyTimeout;
unsigned int minPlayers;
	bool lobbyChanged;
unsigned int lobbySize;

	static const unsigned int maxClients = 50;
	static const unsigned int chatLimit = 3;
	static const unsigned int maxHandSize = 18;
};


#endif /*SERVER_H*/
