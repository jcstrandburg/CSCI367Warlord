/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

clientinterfaceadapter.h

Specification for ClientInterfaceAdapter module, acts as an adapter between the play state representation
used by the Client object and that used by the Interface object
=============================*/

#ifndef CLIENTINTERFACEADAPTER_H_
#define CLIENTINTERFACEADAPTER_H_

#include <string>
#include "interfaceadapter.h"

struct ClientPlayState;

class ClientInterfaceAdapter: public InterfaceAdapter
{
public:
	ClientInterfaceAdapter( ClientPlayState *s );
	~ClientInterfaceAdapter();
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
	ClientPlayState *state;	
};

#endif /*CLIENTINTERFACEADAPTER_H_*/
