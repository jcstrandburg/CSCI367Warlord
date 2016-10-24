/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

interface.h

Spec for a Interface object used a base class for specific interface types
(ie Curses, Text, SDL, etc..)
=============================*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include <deque>
#include <string>
#include <tr1/memory>
#include "card.h"
#include "interfaceadapter.h"

struct InterfaceEvent
{
enum Type
{
	PlayerPassed,
	PlayerSkipped,
	PlayerPlayed,
	NewHand,
	NewRound,
	ChatMessage,
	PlayerEnterLobby,
	PlayerLeaveLobby,
	Notification,
	HandChanged,
	TableUpdate,
};
Type type;
PlayerID plr;
CardList cards;
string msg;
InterfaceEvent();
InterfaceEvent( const Type t, const PlayerID& p, const CardList& c, const string &m);
};

class Interface
{
public:
enum Mode
{
	NoGame,
	InGameInactive,
	InGameActive,
	InGameSwapping,
	Spectating,
};
enum Verbosity
{
	Silent,
	Quiet,
	Verbose,
};

	Interface( Verbosity v);
	virtual ~Interface();
void	AddEvent( const InterfaceEvent::Type t, const PlayerID& p, const CardList& c, const string &m );
void	AddEvent( const InterfaceEvent &e);
void	Update();
void	SetInterfaceAdapter( IfaceAdapterPtr);
virtual void	SetMode( Mode m);
bool	HasChat();
void 	GetChat( string &s);
bool 	HasPlay();
void 	GetPlay( CardList &p);
bool	QuitReceived();
virtual void Trace( const string &s);

protected:

void 	ProcessEvent( const InterfaceEvent &e);
	
virtual void PostUpdate() = 0;
virtual void Render() = 0;

virtual void HandlePlayerPassed( const InterfaceEvent &e) = 0;
virtual void HandlePlayerSkipped( const InterfaceEvent &e) = 0;
virtual void HandlePlayerPlayed( const InterfaceEvent &e) = 0;
virtual void HandleNewHand( const InterfaceEvent &e) = 0;
virtual void HandleNewRound( const InterfaceEvent &e) = 0;
virtual void HandleChatMessage( const InterfaceEvent &e) = 0;
virtual void HandlePlayerEnterLobby( const InterfaceEvent &e) = 0;
virtual void HandlePlayerLeaveLobby( const InterfaceEvent &e) = 0;
virtual void HandleNotification( const InterfaceEvent &e) = 0;
virtual void HandleHandChanged( const InterfaceEvent &e) = 0;
virtual void HandleTableUpdate( const InterfaceEvent &e) = 0;

bool eventReady;//ready to process the next event
deque<InterfaceEvent> events;
deque<string> chats;//pending outgoing chats messages
CardList play;//pending outgoing card selections
bool hasPlay;
IfaceAdapterPtr adapter;
Mode mode;
bool quit;
Verbosity verbosity;
};

InterfaceEvent NotificationEvent( const string &s);//helper function


typedef std::tr1::shared_ptr<Interface> InterfacePtr;

#endif 
