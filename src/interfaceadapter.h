/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

interfaceadapter.h

Spec for a InterfaceAdapter object used a base class for specific interface adapters.
This class exists so that the interface is not tied to a specific game state representation, so the 
server and client can use the same interface code
=============================*/

#ifndef INTERFACEADAPTER_H_
#define INTERFACEADAPTER_H_

#include <string>
#include <tr1/memory>
#include "card.h"

typedef std::string PlayerID;

class InterfaceAdapter
{
public:
enum PlayerStatus
{
	PlayerActive,
	PlayerWaiting,
	PlayerPassed,
	PlayerDisconnected,
	PlayerEmpty,
};
	
	virtual std::string GetPlayerName( unsigned int index) = 0;
	virtual unsigned int GetPlayerHandSize( unsigned int index) = 0;
	virtual PlayerStatus GetPlayerStatus( unsigned int index) = 0;
	virtual unsigned int GetNumPlayers() = 0;
	virtual unsigned int GetNumCards() = 0;
	virtual unsigned int GetPlayerStrikes( unsigned int index) = 0;
	virtual Card	GetCard( unsigned int index) = 0;
	virtual CardList GetLastPlay() = 0;
	virtual string GetLocalName() = 0;
	
private:
	
};

typedef std::tr1::shared_ptr<InterfaceAdapter> IfaceAdapterPtr;

#endif /*INTERFACEADAPTER_H_*/
