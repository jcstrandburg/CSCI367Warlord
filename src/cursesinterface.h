/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

cursesinterface.h

Implementation for the CursesInterface object, which uses the curses library
=============================*/

#ifndef CURSESINTERFACE_H
#define CURSESINTERFACE_H

#include "interface.h"
#include "cursesdisplays.h"

class CursesInterface: public Interface
{
public:

	CursesInterface( Interface::Verbosity v);
	virtual ~CursesInterface();
	virtual void	SetMode( Interface::Mode m);
	void	ParsePlayString( const string &s);
	
protected:
	void InitCurses();
	void ExitCurses();
	void Notify( const string &s);
	
virtual void Trace( const string &s);
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

DisplayBase *tableDisp;
ScrollingDisplay *chatbox;
ListDisplay *lobbyList;
TickerDisplay *notes;
DisplayBase *handDisp;
TextInDisplay *textin;
bool chatMode;
};


#endif 
