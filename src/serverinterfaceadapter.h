/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

serverinterfaceadapter.h

Specification for ClientInterfaceAdapter module, acts as an adapter between the play state
representation used by the Server object and that used by the Interface object
=============================*/

#ifndef SERVERINTERFACEADAPTER_H_
#define SERVERINTERFACEADAPTER_H_

#include <string>
#include "interfaceadapter.h"
#include "serverplaystate.h"

class ServerInterfaceAdapter: public InterfaceAdapter
{
public:
	ServerInterfaceAdapter( PlayState *s );
	~ServerInterfaceAdapter();
	virtual std::string GetPlayerName( unsigned int index);
	virtual unsigned int GetPlayerHandSize( unsigned int index);
	virtual PlayerStatus GetPlayerStatus( unsigned int index);
	virtual unsigned int GetNumPlayers();
	virtual unsigned int GetNumCards();
	virtual unsigned int GetPlayerStrikes( unsigned int index);	
	virtual Card	GetCard( unsigned int index);
	virtual CardList GetLastPlay();	
	virtual string GetLocalName();
	
private:
	PlayState *state;	
};

#endif /*CLIENTINTERFACEADAPTER_H_*/
