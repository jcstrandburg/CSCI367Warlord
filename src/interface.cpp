/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

interface.cpp

Implementation for a Interface object used a base class for specific interface
types (ie Curses, Text, SDL, etc..)
=============================*/

#include <assert.h>
#include <stdio.h>
#include "interface.h"


//help function for creating notification events
InterfaceEvent NotificationEvent( const string &s)
{
	return InterfaceEvent( InterfaceEvent::Notification, "", CardList(), s);
}




InterfaceEvent::InterfaceEvent()
{
	
}


InterfaceEvent::InterfaceEvent( const Type t, const PlayerID& p, const CardList& c, const string &m):
	type( t), plr( p), cards( c), msg( m)
{
	
}


Interface::Interface( Verbosity v):
	eventReady( true), hasPlay( false), mode( Interface::NoGame), quit( false), verbosity( v)
{
	
}
	
Interface::~Interface()
{
}

void	Interface::AddEvent( const InterfaceEvent::Type t, const PlayerID& p, const CardList& c, const string &m )
{
	events.push_back( InterfaceEvent( t, p, c, m));
}

void	Interface::AddEvent( const InterfaceEvent &e )
{
	events.push_back( e);
}

void	Interface::Update()
{
	while ( events.size() > 0 && eventReady )
	{
		ProcessEvent( events.front());
		events.pop_front();
	}
	
	PostUpdate();
}

void 	Interface::SetInterfaceAdapter( IfaceAdapterPtr p)
{
	adapter = p;
}

void	Interface::SetMode( Interface::Mode m)
{
	mode = m;
}

bool	Interface::HasChat()
{
	return chats.size() > 0;
}

void 	Interface::GetChat( string &s)
{
	assert( chats.size() > 0 );
	s = chats.front();
	chats.pop_front();
}

bool 	Interface::HasPlay()
{
	return hasPlay;
}

void 	Interface::GetPlay( CardList &p)
{
	assert( hasPlay);
	p = play;
	hasPlay = false;
}

void 	Interface::ProcessEvent( const InterfaceEvent &e)
{
	switch (e.type)
	{
	case InterfaceEvent::PlayerPassed:
		HandlePlayerPassed( e);
		break;
	case InterfaceEvent::PlayerSkipped:
		HandlePlayerSkipped( e);
		break;		
	case InterfaceEvent::PlayerPlayed:
		HandlePlayerPlayed( e);
		break;
	case InterfaceEvent::NewHand:
		HandleNewHand( e);
		break;
	case InterfaceEvent::NewRound:
		HandleNewRound( e);
		break;
	case InterfaceEvent::ChatMessage:
		HandleChatMessage( e);
		break;
	case InterfaceEvent::PlayerEnterLobby:
		HandlePlayerEnterLobby( e);
		break;
	case InterfaceEvent::PlayerLeaveLobby:
		HandlePlayerLeaveLobby( e);
		break;
	case InterfaceEvent::Notification:
		HandleNotification( e);
		break;
	case InterfaceEvent::HandChanged:
		HandleHandChanged( e);
		break;
	case InterfaceEvent::TableUpdate:
		HandleTableUpdate( e);
		break;
	}
}

bool	Interface::QuitReceived()
{
	return quit;
}

void	Interface::Trace( const string &s)
{
	printf( "%s\n", s.c_str());
}

