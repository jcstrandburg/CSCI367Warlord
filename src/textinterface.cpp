/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

textinterface.cpp

Implementation for the TextInterface object, which uses only basic console output
=============================*/

#include <stdio.h>

#include "textinterface.h"
#include "input.h"
#include "netmessage.h"

const string ChatPrompt = "Chat";
const string CardPrompt = "Enter play";
const string SwapPrompt = "Enter swap card";

TextInterface::TextInterface( Interface::Verbosity v):
	Interface( v)
{
}

TextInterface::~TextInterface()
{
}

void	TextInterface::SetMode( Interface::Mode m)
{
	//since this method is potentially blocking, call update first to process any other events that have come in
	Update();	
	
	if ( m == Interface::InGameActive )
	{
	string s;

		while ( !hasPlay)
		{
			PrintHand();
			printf( "Select cards to play: ");
			GetLine( s);
			ParsePlayString( s);
			if ( !hasPlay)
			{
				printf( "Invalid selection!\n");
			}
		}
	}	
	else if ( m == Interface::InGameSwapping)
	{
	string s;
	
	while ( !hasPlay)
	{
		PrintHand();
		printf( "Select a card to swap: ");
		GetLine( s);
		ParsePlayString( s);
		if ( !hasPlay)
		{
			printf( "Invalid selection!\n");
		}
	}
	}
	
	Interface::SetMode( m);
}

void TextInterface::Trace( const string &s)
{
	if ( verbosity != Interface::Silent)
		printf( "%s\n", s.c_str());
}

void TextInterface::PostUpdate()
{
string s;	
int c;	

	//if the textin box has not captured input, read it now
	if ( KeyPressed())
	{
		c = GetChar();
		if ( c == 27)
		{
			Trace( "Exiting");
			quit = true;
			return;
		}
		//else if ( c == )

	}
}

void TextInterface::PrintHand()
{
unsigned i;	
	
	printf( "Your hand: {");
	for ( i = 0; i < adapter->GetNumCards(); i++)
	{
		printf( " %s", adapter->GetCard( i).CardToString().c_str());
	}
	
	printf( "}\n");
}

void 	TextInterface::ParsePlayString( const string &s)
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

void TextInterface::Render()
{
	
}

void TextInterface::HandlePlayerPassed( const InterfaceEvent &e)
{
	Trace( "player passed");
}

void TextInterface::HandlePlayerSkipped( const InterfaceEvent &e)
{
	Trace( "player skipped");
}

void TextInterface::HandlePlayerPlayed( const InterfaceEvent &e)
{
	Trace( "player played");
}

void TextInterface::HandleNewHand( const InterfaceEvent &e)
{
	Trace( "new hand");
}

void TextInterface::HandleNewRound( const InterfaceEvent &e)
{
	Trace( "new round");	
}

void TextInterface::HandleChatMessage( const InterfaceEvent &e)
{
	if ( verbosity != Interface::Silent)
		printf( "%s: %s\n", e.plr.c_str(), e.msg.c_str());
}

void TextInterface::HandlePlayerEnterLobby( const InterfaceEvent &e)
{
}

void TextInterface::HandlePlayerLeaveLobby( const InterfaceEvent &e)
{
}

void TextInterface::HandleNotification( const InterfaceEvent &e)
{
	if ( verbosity == Interface::Verbose)
		printf( "Notification: %s\n", e.msg.c_str());
}

void TextInterface::HandleHandChanged( const InterfaceEvent &e)
{
}

void TextInterface::HandleTableUpdate( const InterfaceEvent &e)
{
}

