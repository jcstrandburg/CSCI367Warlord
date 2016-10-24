/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

clientinterfaceadapter.cpp

Implementation for ClientInterfaceAdapter module, acts as an adapter between the play state
representation used by the Client object and that used by the Interface object
=============================*/
#include <assert.h>

#include "clientinterfaceadapter.h"
#include "clientplaystate.h"

using namespace std;

ClientInterfaceAdapter::ClientInterfaceAdapter( ClientPlayState *s):
	state( s)
{
	
}

ClientInterfaceAdapter::~ClientInterfaceAdapter()
{
	
}

string ClientInterfaceAdapter::GetPlayerName( unsigned int index)
{
	assert( state);
	return state->players[index].name;
}

unsigned int ClientInterfaceAdapter::GetPlayerHandSize( unsigned int index)
{
	assert( state);
	return state->players[index].handSize;
}

InterfaceAdapter::PlayerStatus ClientInterfaceAdapter::GetPlayerStatus( unsigned int index)
{
	assert( state);
	
	switch ( state->players[index].status)
	{
	case ClientPlayerState::Active:
		return InterfaceAdapter::PlayerActive;
		break;
	case ClientPlayerState::Waiting:
		return InterfaceAdapter::PlayerWaiting;
		break;
	case ClientPlayerState::Passed:
		return InterfaceAdapter::PlayerPassed;
		break;
	case ClientPlayerState::Disconnected:
		return InterfaceAdapter::PlayerDisconnected;
		break;
	case ClientPlayerState::Empty:
		return InterfaceAdapter::PlayerEmpty;
		break;
	}
	
	return InterfaceAdapter::PlayerWaiting;
}

unsigned int ClientInterfaceAdapter::GetPlayerStrikes( unsigned int index)
{
	return state->players[index].strikes;
}

unsigned int ClientInterfaceAdapter::GetNumPlayers()
{
	assert( state);
	return state->numPlayers;
}

unsigned int ClientInterfaceAdapter::GetNumCards()
{
	assert( state);
	return state->localHand.Size();
}

Card	ClientInterfaceAdapter::GetCard( unsigned int index)
{
	assert( state);
	assert( index < state->localHand.Size());
	return state->localHand[index];
}

CardList 	ClientInterfaceAdapter::GetLastPlay()
{
	return state->lastPlay;
}

string 	ClientInterfaceAdapter::GetLocalName()
{
	return state->localName;
}
