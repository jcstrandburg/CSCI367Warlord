/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

textinterface.h

Specification for the TextInterface object, which uses only basic console output
=============================*/

#ifndef TEXTINTERFACE_H
#define TEXTINTERFACE_H

#include "interface.h"

class TextInterface: public Interface
{
public:

	TextInterface( Interface::Verbosity v);
	virtual ~TextInterface();

virtual void	SetMode( Interface::Mode m);
virtual void Trace( const string &s);

protected:
	
	void PrintHand();
	void ParsePlayString( const string &s);

virtual void PostUpdate();
virtual void Render();
virtual void HandlePlayerPassed( const InterfaceEvent &e);
virtual void HandlePlayerSkipped( const InterfaceEvent &e);
virtual void HandlePlayerPlayed( const InterfaceEvent &e);
virtual void HandleNewHand( const InterfaceEvent &e);
virtual void HandleNewRound( const InterfaceEvent &e);
virtual void HandleChatMessage( const InterfaceEvent &e);
virtual void HandlePlayerEnterLobby( const InterfaceEvent &e);
virtual void HandlePlayerLeaveLobby( const InterfaceEvent &e);
virtual void HandleNotification( const InterfaceEvent &e);
virtual void HandleHandChanged( const InterfaceEvent &e);
virtual void HandleTableUpdate( const InterfaceEvent &e);
};


#endif 
