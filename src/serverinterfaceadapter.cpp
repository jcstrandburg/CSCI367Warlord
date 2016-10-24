/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

serverinterfaceadapter.cpp

Implementation for ServerInterfaceAdapter module, acts as an adapter between the play
state representation used by the Server object and that used by the Interface object
=============================*/
#include <assert.h>

#include "serverinterfaceadapter.h"
#include "serverplaystate.h"

using namespace std;

ServerInterfaceAdapter::ServerInterfaceAdapter( PlayState *s):
	state( s)
{
	
}

ServerInterfaceAdapter::~ServerInterfaceAdapter()
{
	
}

string ServerInterfaceAdapter::GetPlayerName( unsigned int index)
{
	assert( state);
	return state->players[index].client->GetIdentifier();
}

unsigned int ServerInterfaceAdapter::GetPlayerHandSize( unsigned int index)
{
	assert( state);
	return state->players[index].hand.Size();
}

InterfaceAdapter::PlayerStatus ServerInterfaceAdapter::GetPlayerStatus( unsigned int index)
{
	assert( state);
	
	switch ( state->players[index].status)
	{
	case PlayerState::Active:
		return InterfaceAdapter::PlayerActive;
		break;
	case PlayerState::Waiting:
		return InterfaceAdapter::PlayerWaiting;
		break;
	case PlayerState::Passed:
		return InterfaceAdapter::PlayerPassed;
		break;
	case PlayerState::Disconnected:
		return InterfaceAdapter::PlayerDisconnected;
		break;
	case PlayerState::Empty:
		return InterfaceAdapter::PlayerEmpty;
		break;
	}
	
	return InterfaceAdapter::PlayerWaiting;
}

unsigned int ServerInterfaceAdapter::GetPlayerStrikes( unsigned int index)
{
	return state->players[index].client->GetStrikes();
}

unsigned int ServerInterfaceAdapter::GetNumPlayers()
{
	assert( state);
	return state->numPlayers;
}

unsigned int ServerInterfaceAdapter::GetNumCards()
{
	assert( state);
	return 0;
}

Card	ServerInterfaceAdapter::GetCard( unsigned int index)
{
	throw "ServeerInterfaceAdapter::GetCard called, no bueno";
}

CardList 	ServerInterfaceAdapter::GetLastPlay()
{
	return state->lastPlay;
}

string 	ServerInterfaceAdapter::GetLocalName()
{
	return "";
}
