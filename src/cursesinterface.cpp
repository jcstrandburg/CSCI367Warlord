/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

cursesinterface.cpp

Implementation for the CursesInterface object, which uses the curses library
=============================*/

#include <stdio.h>
#include <curses.h>

#include "cursesinterface.h"
#include "input.h"
#include "netmessage.h"
#include "netmessage.h"

const string ChatPrompt = "Chat";
const string CardPrompt = "Enter play";
const string SwapPrompt = "Enter swap card";

void drawRect( WINDOW *win, int x1, int y1, int x2, int y2)
{
static const int vchar = ACS_VLINE;//186;
static const int hchar = ACS_HLINE;//205;
static const int corner = '@';//177;
	
	//the lines
	mvwhline( win, y1, x1, hchar, x2-x1+1);
	mvwhline( win, y2, x1, hchar, x2-x1+1);
	mvwvline( win, y1+1, x1, vchar, y2-y1-1);
	mvwvline( win, y1+1, x2, vchar, y2-y1-1);
	
	//the corners
	mvwaddch( win, y1, x1, corner);
	mvwaddch( win, y1, x2, corner);
	mvwaddch( win, y2, x1, corner);
	mvwaddch( win, y2, x2, corner);
}

CursesInterface::CursesInterface( Interface::Verbosity v):
	Interface( v), chatMode( false)
{
	InitCurses();
}

CursesInterface::~CursesInterface()
{
	ExitCurses();
}

void	CursesInterface::SetMode( Interface::Mode m)
{
	if ( m == Interface::InGameActive )
	{
		textin->Activate( CardPrompt);
		chatMode = false;
	}	
	else if ( m == Interface::InGameSwapping)
	{
		textin->Activate( SwapPrompt);
		chatMode = false;
	}
	else if ( m == Interface::InGameInactive)
	{
		if ( !chatMode)
		{
			textin->Deactivate();
		}
	}
	Interface::SetMode( m);
}

void 	CursesInterface::ParsePlayString( const string &s)
{
vector<string> tokens;
unsigned int i;
Card c;
CardList l;	
string s2;

	if ( s.size() == 0)
	{
		hasPlay = false;
		return;
	}
	
	if ( s == "pass")
	{
		hasPlay = true;
		play.Clear();
		return;
	}
	
	ExplodeString( " ", s, tokens);
	for ( i = 0; i < tokens.size(); i++)
	{
		c = tokens[i];
		if ( c == NoCard)
		{
			s2 = "Invalid card: " + tokens[i];
			Trace( s2);
			return;
		}
		else
		{
			l.Add( c);
		}
	}
	
	play = l;
	hasPlay = true;
}

void	CursesInterface::InitCurses()
{
int h, w;
	
	//start curses
	initscr();
	savetty();
	start_color();
	refresh();
	noecho();
	keypad( stdscr, true);
	/*curs = */curs_set( 0);
	init_pair( 1, COLOR_CYAN, COLOR_BLACK);
	init_pair( 2, COLOR_WHITE, COLOR_BLACK);

	//draw the frame
	getmaxyx( stdscr, h, w);	
	attron( COLOR_PAIR( 1));
	drawRect( stdscr, 0, 0, w-1, h-1);
	drawRect( stdscr, w-10, 0, w-1, h-3);
	drawRect( stdscr, 0, h-5, w-10, h-3);
	drawRect( stdscr, 0, 9, w-10, h-7);
	attroff( COLOR_PAIR( 1));
	refresh();

	//create the displays
	chatbox = new ScrollingDisplay( 1, 10, w-11, h-17);	
	tableDisp = new DisplayBase( 1, 1, w-11, 8);
	handDisp = new DisplayBase( 1, h-2, w-2, 1);
	notes = new TickerDisplay( 1, h-4, w-11);
	textin = new TextInDisplay( 1, h-6, w-11);
	lobbyList = new ListDisplay( w-9, 1, 8, h-4);
}

void	CursesInterface::ExitCurses()
{
	//kill the displays
	delete tableDisp;
	delete chatbox;
	delete lobbyList;
	delete notes;
	delete handDisp;
	delete textin;
	
	//kill curses
	clear();
	refresh();
	resetty();
	endwin();	
}

void CursesInterface::Notify( const string &s)
{
	if ( verbosity == Interface::Verbose)
	{
		notes->Add( s);
	}
}

void CursesInterface::Trace( const string &s)
{
	if ( verbosity != Interface::Silent)
		chatbox->Output( s);
}

void CursesInterface::PostUpdate()
{
string s;	
int c;	

	if ( textin->HasInput())
	{
		textin->GetInput( s);
		if ( chatMode)
		{
			//HandleChatMessage( InterfaceEvent( InterfaceEvent::ChatMessage, adapter->GetLocalName(), CardList(), s));
			chats.push_back( s);
			chatMode = false;
		}
		else
		{
			if ( s == "quit")
			{
				quit = true;
				return;
			}

			ParsePlayString( s);
			if ( hasPlay)
				SetMode( Interface::InGameInactive);
			else
				textin->Activate( CardPrompt);
		}
	}
	//the player esc'd then HasInput will not be true but the textin will be inactive, so we need to reactivate it
	else if ( !textin->Active() )
	{
		if ( mode == Interface::InGameActive)
		{
			textin->Activate( CardPrompt);
		}
		else if ( mode == Interface::InGameSwapping)
		{
			textin->Activate( SwapPrompt);
		}

	}

	//if the textin box has not captured input, read it now
	if ( KeyPressed() && !textin->Active() )
	{
		c = getch();
		if ( c == 27)
		{
			Trace( "Exiting");
			quit = true;
			return;
		}
		else if ( c == 13 || c == 10 || c == 459)
		{
			textin->Activate( ChatPrompt);
			chatMode = true;
		}
	}

	notes->Update();
	textin->Update();
}

void CursesInterface::Render()
{
	
}

void CursesInterface::HandlePlayerPassed( const InterfaceEvent &e)
{
	Trace( "player passed");
}

void CursesInterface::HandlePlayerSkipped( const InterfaceEvent &e)
{
	Trace( "player skipped");
}

void CursesInterface::HandlePlayerPlayed( const InterfaceEvent &e)
{
	Trace( "player played");
}

void CursesInterface::HandleNewHand( const InterfaceEvent &e)
{
	Trace( "new hand");
}

void CursesInterface::HandleNewRound( const InterfaceEvent &e)
{
	Trace( "new round");	
}

void CursesInterface::HandleChatMessage( const InterfaceEvent &e)
{
	Trace( e.plr + ": " + e.msg);
}

void CursesInterface::HandlePlayerEnterLobby( const InterfaceEvent &e)
{
	lobbyList->Add( e.plr);
}

void CursesInterface::HandlePlayerLeaveLobby( const InterfaceEvent &e)
{
	lobbyList->Remove( e.plr);
}

void CursesInterface::HandleNotification( const InterfaceEvent &e)
{
	Notify( e.msg);
}

void CursesInterface::HandleHandChanged( const InterfaceEvent &e)
{
string s;
unsigned int i;

	s = "{";
	for ( i = 0; i < adapter->GetNumCards(); i++)
	{
		if ( i != 0)
			s += ' ';
		s += adapter->GetCard( i).CardToString();
	}
	s += "}";
	handDisp->SetLine( 0, s);
	if ( e.msg.size() > 0 )
		Notify( e.msg);
}

void CursesInterface::HandleTableUpdate( const InterfaceEvent &e)
{
unsigned int i, j;	
CardList play;
string s;

	for ( i = 0; i < adapter->GetNumPlayers(); i++)
	{
		s = PadString( adapter->GetPlayerName( i), 9);
		for ( j = 0; j < adapter->GetPlayerStrikes( i) && j < 3; j++)
			s += 'X';
		s += '\t';
		switch ( adapter->GetPlayerStatus(i))
		{
		case InterfaceAdapter::PlayerActive:
			s += "Active";
			break;
		case InterfaceAdapter::PlayerWaiting:
			s += "Waiting";
			break;
		case InterfaceAdapter::PlayerPassed:
			s += "Passed ";
			break;
		default:
			s += "XXXXXXX";
		}
		s += '\t';
		s += UnsignedToString( adapter->GetPlayerHandSize( i), 2);
		s += " cards";
		
		if ( adapter->GetPlayerName(i) == adapter->GetLocalName() )
			tableDisp->SetLine( i, s, true);
		else
			tableDisp->SetLine( i, s);
	}
	for ( i = adapter->GetNumPlayers(); i < 7; i++)
	{
		tableDisp->SetLine( i, "");
	}
	
	play = adapter->GetLastPlay();
	s = UnsignedToString( play.Size(), 1) + " Cards on the table: {";
	for ( i = 0; i < play.Size(); i++)
	{
		s += " "+play[i].CardToString()+"("+UnsignedToString( (unsigned)play[i],1)+")";
	}
	s += "}";
	tableDisp->SetLine( 7, s);
}

