/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

client.cpp

Implementation for Client module, which provides the foundation of the functionality of the client application
=============================*/

#include <iostream>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "warlord.h"
#include "wlconnection.h"
#include "client.h"
#include "netmanager.h"
#include "netmessage.h"
#include "input.h"
#include "interface.h"
#include "clientplaystate.h"
#include "clientinterfaceadapter.h"
#include "timer.h"

using namespace std;

const unsigned int autoModeDelay = 5;



//================================================
//Client class definition
//================================================

Client::Client( InterfacePtr i, bool doAutoMode, bool doLogging):
	exitCondition( false), iface( i) , task( NoTask), invalidPlay( false), autoMode( doAutoMode),
	lostConnection( false)
{
InterfaceAdapter *a = new ClientInterfaceAdapter( &state);
IfaceAdapterPtr p( a);	
	
	iface->SetInterfaceAdapter( p);
	
	if ( autoMode)
		iface->Trace( "Going into auto-mode!");
	if ( doLogging)
	{
		iface->Trace( "Logging all socket traffic");
		net.StartLogging( "clogs");
	}
	srand( time(0));
}

Client::~Client()
{
	
}

//Performs some basic validation on the given card list and then builds a cplay message
//from it to send off to the server
bool Client::AttemptPlay( CardList &c)
{
unsigned i;	
NetMessage msg;
string s;

	//make sure we have all the cards in our hand
	for ( i = 0; i < c.Size(); i++)
	{
		if ( !state.localHand.Contains( c[i]))
			return false;
	}
	
	//do more error checking eventually
	//loblobloblobloblob
	
	//if we get here we should be good, go ahead and send the message and return true
	msg.Reset( NetMessage::CPlay);
	s = "";
	for ( i = 0; i < c.Size(); i++)
	{
		if ( i != 0)
			s += ",";
		s += UnsignedToString( (unsigned int)c[i], 2);
		state.localHand.Remove( c[i]);
	}
	for ( i = c.Size(); i < 4; i++)
	{
		if ( i != 0)
				s += ",";
		s += UnsignedToString( (unsigned int)NoCard, 2);
	}
	msg.AddVariable( s);
	conn.SendMessage( msg);
	return true;
}

//attempts to create a connection to a server at the given address
void Client::Connect( const string &host, int port)
{
	try
	{
		conn = net.RequestConnection( host.c_str(), port);
		conn.SetMaxMessageLength( 400);
	}
	catch (const char *s)
	{
		//if we're in auto mode just keep throwing the exception
		if ( autoMode)
		{
			throw s;
		}
		//otherwise display it in the interface for the player
		else
		{
		string out( "Exception: ");
			iface->Trace( out + s);
		}
	}
}

//attempts to create a connection to a server at the given address and then
//sends a cjoin message
void	Client::ConnectAs( const std::string &addr, int port, const std::string &name)
{
NetMessage msg;

	Connect( addr, port);
	msg.Reset( NetMessage::CJoin);
	msg.AddVariable( PadString( name, clientNameLength));
	conn.SendMessage( msg);
}

bool Client::ExitCondition()
{
	return exitCondition;
}

//basic updating method, checks for messages from the server and the interface and whatnot
void Client::Update()
{
string s;
NetMessage msg;
CardList play;
	
	net.Update();
	if ( iface->QuitReceived() )
	{
		exitCondition = true;
		return;
	}
	
	if ( !conn.IsOpen())
	{
		if ( !lostConnection)
		{
			lostConnection = true;
			iface->Trace( "Connection closed!");
		}
		if ( autoMode)
			exitCondition = true;
		return;
	}
	
	//do some task dependant processing
	switch ( task)
	{
	case NoTask:
		break;
	case AutoPlay:

		//check to see if we need to generate an automatic play
		if ( autoTimer.GetMsec() > autoModeDelay )
		{
		CardList playme;

			AutoSelectCards( playme);
			AttemptPlay( playme);
			iface->AddEvent( InterfaceEvent::HandChanged, "", CardList(), "");
			task = NoTask;
		}
		break;
	case GetPlay:

		//see if the interface has a play for us
		if ( iface->HasPlay() )
		{
			iface->GetPlay( play);
			if ( AttemptPlay( play) == true )
			{
				iface->AddEvent( InterfaceEvent::HandChanged, "", CardList(), "");
				task = NoTask;
			}
			else
			{
				iface->AddEvent( NotificationEvent( "Invalid play!"));
				iface->SetMode( Interface::InGameActive);
			}
		}
		break;
	case GetSwap:

		//see if the interface has a "play" to swap
		if ( iface->HasPlay() )
		{
			iface->GetPlay( play);

			//validate the size of the card selection, then send it off to the server if it's ok
			if ( play.Size() != 1 )
			{
				iface->AddEvent( NotificationEvent( "Invalid swap!"));
				iface->SetMode( Interface::InGameSwapping);
			}
			else
			{
				if ( play[0] != NoCard)
				{
				NetMessage msg;
				
					msg.Reset( NetMessage::CSwap);
					msg.AddVariable( UnsignedToString( (unsigned int)play[0], 2));
					conn.SendMessage( msg);
					task = NoTask;
				}
				else
				{
					iface->AddEvent( NotificationEvent( "Invalid swap!"));
					iface->SetMode( Interface::InGameActive);					
				}
			}
		}		
		break;
	}
	
	//check for chat messages from the interface to send off to the server
	while ( iface->HasChat())
	{
	string s;
		iface->GetChat( s);

		if ( s == "chand")
		{
			msg.Reset( NetMessage::CHand);
			conn.SendMessage( msg);
		}
		else
		{
			msg.Reset( NetMessage::CChat);
			msg.AddVariable( PadString( s, 63));
			conn.SendMessage( msg);
		}
	}
	
	//look for messages from the server
	while ( conn.GetMessageStatus() != WLConnection::NoMessage)
	{
		switch ( conn.GetMessageStatus() )
		{
		case WLConnection::GoodMessage:
			conn.GetMessage( msg);
			ProcessMessage( msg);
			break;
		case WLConnection::LengthExceeded:
			throw "Length exceeded";
			break;
		case WLConnection::Malformed:
			throw "Malformed";
			break;
		case WLConnection::NoMessage://should never get here, just needed to avoid compiler warnings
			break;
		}
	}
	
	//if while processesing messages we got a strike for an invalid play, go back to the interface and get a new play
	if ( invalidPlay)
	{
		invalidPlay = false;
		task = GetPlay;
		iface->SetMode( Interface::InGameActive);
	}
}

//Processes an individual message from the derver
void Client::ProcessMessage( const NetMessage &msg)
{
	switch ( msg.type)
	{
	case NetMessage::SChat:
		iface->AddEvent( InterfaceEvent::ChatMessage, msg.vars[0], CardList(), msg.vars[1]);
		break;
	case NetMessage::SHand:
		ProcessHandMessage( msg);
		break;
	case NetMessage::SJoin:
		if ( msg.vars.size() != 1)
		{
			throw "Invalid sjoin message!";
		}
		else
		{
		string s = "Welcome to the server " + msg.vars[0] + ".";

			state.localName = msg.vars[0];
			iface->SetMode( Interface::Spectating);
			iface->Trace( s);
		}
		break;
	case NetMessage::SLobb:
		ProcessLobbyMessage( msg);
		break;
	case NetMessage::STabl:
		ProcessTableMessage( msg);
		break;
	case NetMessage::STrik:
		ProcessStrikeMessage( msg);
		break;
	case NetMessage::SWaps:
		if ( msg.vars.size() != 2)
			throw "Malformed Swaps message!";
		else
		{
		int cardVal1, cardVal2;
		Card card1, card2;

			sscanf( msg.vars[0].c_str(), "%d", &cardVal1);
			sscanf( msg.vars[1].c_str(), "%d", &cardVal2);
			card1 = cardVal1;
			card2 = cardVal2;

			iface->AddEvent( NotificationEvent( "The warlord took your " + card2.CardToString() + " and gave you " +  card1.CardToString() + "."));
			iface->AddEvent( InterfaceEvent::HandChanged, "", CardList(), "");
		}
		break;
	case NetMessage::SWapw:
		if (msg.vars.size() != 1)
		{
			throw "Malformed swapw message!";
		}
		else
		{
		int cardval;

			sscanf( msg.vars[0].c_str(), "%d", &cardval);
			swapCard = cardval;
			SwapActivation();
		}
		break;
	default:
		iface->Trace( "unhandled message: " + msg.Serialize());
		break;
	}
}

void	Client::ProcessHandMessage( const NetMessage& msg)
{
unsigned int i;
unsigned int cardval;
CardList hand2;
vector<string> tokens;

	ExplodeString( ",", msg.vars[0], tokens);
	//build a hand from the given message
	for ( i = 0; i < tokens.size(); i++)
	{
		sscanf( tokens[i].c_str(), "%u", &cardval);
		hand2.Add( Card(cardval));
	}

	state.localHand = hand2;
	iface->AddEvent( InterfaceEvent::HandChanged, "", CardList(), "Hand Updated");
}

void 	Client::ProcessLobbyMessage( const NetMessage& msg)
{
unsigned int i;
PlayerList newlobby;
PlayerList::iterator iter;
bool found = false;
vector<string> tokens;
int numPlayers;

	if ( sscanf( msg.vars[0].c_str(), "%d", &numPlayers) < 1)
	{
		iface->Trace( "Malformed lobby message received from server!");
		return;
	}

	if ( numPlayers > 0)
	{
		ExplodeString( ",", msg.vars[1], tokens);

		//build the new lobby list plus check to see if there are new entries
		//plus if we see our name over there and we though we were in game we need to clear the play state
		for ( i = 0; i < tokens.size(); i++)
		{
			newlobby.insert( tokens[i]);
			if ( lobby.find( tokens[i]) == lobby.end() )
			{
				lobby.insert( tokens[i]);
				iface->AddEvent( InterfaceEvent::PlayerEnterLobby, tokens[i], CardList(), "lobby+: " + tokens[i]);
			}
		}
	}

	//check the new lobby list for missing entries
	iter = lobby.begin();
	while ( iter != lobby.end() )
	{
		found = newlobby.find( *iter) != newlobby.end();
		if ( !found)
		{
			//if we get here then this player has disconnected at some point
			iface->AddEvent( InterfaceEvent::PlayerLeaveLobby, *iter, CardList(), "lobby-: " + *iter);
			lobby.erase( iter);
		}
		iter++;
	}
}

void	Client::ProcessTableMessage( const NetMessage &msg)
{
ClientPlayState newState;
unsigned int i;
string s;
	
	UnpackTableStatus( msg, state);
	iface->AddEvent( InterfaceEvent::TableUpdate, "", CardList(), "");
	
	//check to see if the local player is the active player
	for ( i = 0; i < state.players.size(); i++)
	{
		if ( state.players[i].name == state.localName)
		{
			if ( state.players[i].status == ClientPlayerState::Active )
			{
				LocalTurnActivation();
			}
			else if ( task != GetSwap ) 
			{
				iface->SetMode( Interface::InGameInactive);				
			}
		}
	}	
}

void	Client::ProcessStrikeMessage( const NetMessage &msg)
{

	if ( msg.vars.size() != 2)
		throw "Malformed strike message received!";
	else
	{
	string s = "Strike #" + msg.vars[1] + "!(" + msg.vars[0] + ") ";
	int e;
	Errors::ErrorCode errcode;
	
		sscanf( msg.vars[0].c_str(), "%d", &e);
		errcode = (Errors::ErrorCode)e;
		
		switch ( errcode)
		{
		case Errors::IllegalPlay:
		case Errors::DifferingFaces:
		case Errors::DuplicatedCard:
			iface->AddEvent( NotificationEvent( s + "Invalid Play"));
			break;
		case Errors::LowCardCount:
			iface->AddEvent( NotificationEvent( s + "Not enough cards"));
			break;
		case Errors::LowFaceValue:			
			iface->AddEvent( NotificationEvent( s + "Card(s) too low"));
			break;
		case Errors::InvalidFirstTurn:
			iface->AddEvent( NotificationEvent( s + "Must play C3 on first turn. "));
			break;
		case Errors::CardNotInHand:
			iface->AddEvent( NotificationEvent( s + "You do not have that card"));
			break;
		case Errors::OutOfTurn:
			iface->AddEvent( NotificationEvent( s + "It's not your turn!"));
			break;
		case Errors::PassOnStart:
			iface->AddEvent( NotificationEvent( s + "You cannot pass on the first turn!"));
			break;
		case Errors::Timeout:
			iface->AddEvent( NotificationEvent( s + "Timeout!"));
			break;
		case Errors::BadMessage:
		case Errors::NotInGame:
		case Errors::ExcededLength:
		case Errors::UnknownMsgType:
		case Errors::MalformedMessage:
			iface->AddEvent( NotificationEvent( s + "Bad message!"));
			break;
		case Errors::ChatFlood:
			iface->AddEvent( NotificationEvent( s + "Chat flood."));
			break;
		case Errors::IllegalSwap:
			iface->AddEvent( NotificationEvent( s + "Illegal swap"));
			break;
		case Errors::CannotConnect:
		case Errors::ServerFull:
			iface->AddEvent( NotificationEvent( s + "Cannot connect to server."));
			break;
		default:
			iface->AddEvent( NotificationEvent( s));
			break;
		}

		if ( errcode >= Errors::IllegalPlay && errcode < Errors::Timeout )
		{
			LocalTurnActivation();
		}
	}
}


void	Client::UnpackTableStatus( const NetMessage& msg, ClientPlayState& s)
{
vector<string> v;
unsigned int i;
char c;
	
	if ( msg.vars.size() != 3)
			throw "Invalid number of args in table status message";
	
	ExplodeString( ",", msg.vars[0], v);
	if ( v.size() != 7)
		throw "Invalid <player_list> in table status message";
	
	//get the players
	for ( i = 0; i < v.size(); i++)
	{
	vector<string> tokens;

		ExplodeString( ":", v[i], tokens);

		sscanf( tokens[0].substr( 0, 1).c_str(), "%c", &c);
		sscanf( tokens[0].substr( 1, 4).c_str(), "%u", &(state.players[i].strikes));
		state.players[i].name = RemovePadding( tokens[1]);
		sscanf( tokens[2].c_str(), "%d", &(state.players[i].handSize));
		
		switch ( c)
		{
		case 'a':
		case 'A':
			state.players[i].status = ClientPlayerState::Active;
			break;
		case 'e':
		case 'E':
			state.players[i].status = ClientPlayerState::Empty;
			break;
		case 'w':
		case 'W':
			state.players[i].status = ClientPlayerState::Waiting;
			break;
		case 'd':
		case 'D':
			state.players[i].status = ClientPlayerState::Disconnected;
			break;
		case 'p':
		case 'P':
			state.players[i].status = ClientPlayerState::Passed;
			break;
		}
		
		if ( state.players[i].status != ClientPlayerState::Empty)
			state.numPlayers = i+1;
	}
	
	ExplodeString( ",", msg.vars[1], v);
	state.lastPlay.Clear();
	for ( i = 0; i < v.size(); i++)
	{
	Card c;
	int cardval;
	
		sscanf( v[i].c_str(), "%d", &cardval);
		c = cardval;
		if ( c != NoCard )
			state.lastPlay.Add( c);
	}
	
	if ( msg.vars[2] == "1")
		state.firstGame = true;
	else
		state.firstGame = false;
}


//Generates a play for auto mode. This is done by finding the lowest card denomination for which
//we have enough cards to play, then randomly selecting a number of cards to play that is
//at least as high as the minimum number of cards required, and up to as many cards as we have
void	Client::AutoSelectCards( CardList &dst)
{
unsigned int i, j, k;
Card b;

	dst.Clear();
	//see if we can play whatever we want
	if ( state.lastPlay.Size() == 0)
	{
		if ( state.localHand.Size() > 0)
		{
			dst.Add( state.localHand[0]);
		}
	}
	else
	{
		i = j = 0;
		b = state.lastPlay[0];
		while ( i < state.localHand.Size())
		{
			//if we found a 2 just grab that and skidaddle
			if ( state.localHand[i].GetFaceValue() == Card::Face2)
			{
				dst.Add( state.localHand[i]);
				return;
			}
			//we found at least one card that will work, check the next ones in line
			else if ( state.localHand[i].GetFaceValue() >= b.GetFaceValue() )
			{
				j = 1;
				//count how many cards in a row have the same face value
				while ( (i+j) < state.localHand.Size() && state.localHand[i+j].GetFaceValue() == state.localHand[i].GetFaceValue() )
				{
					j++;
				}

				//if we have enough, randomly decide if we are going to play more than we need
				//then grab that many cards and scram
				if ( j >= state.lastPlay.Size() )
				{
				unsigned int l;

					l = state.lastPlay.Size();
					if ( j > state.lastPlay.Size() )
					{
						l += rand()%(j - state.lastPlay.Size() +1);
					}

					for ( k = 0; k < l; k++)
					{
						dst.Add( state.localHand[i+k]);
					}
					return;
				}
				//not enough cards, skip forward by however many we counted and keep going
				else
				{
					i += j;
				}
			}
			//this cards no good, keep going
			else
			{
				i++;
			}
		}
	}
}

//This method does whatever needs to be done when the client sees that it is our turn.
//Specifically this method checks if we are in auto mode and sets our task appropriately
void Client::LocalTurnActivation()
{
	if ( autoMode)
	{
		task = AutoPlay;
		autoTimer.Restart();
	}
	else
	{
		iface->AddEvent( NotificationEvent( "Your turn."));
		task = GetPlay;
		iface->SetMode( Interface::InGameActive);
	}
}

//This method does whatever needs to be done when the client sees that it is time for us to swap.
//Specifically this method checks if we are in auto mode and either sets our task to swapping
//or automatically sends our lowest card back to swap
void Client::SwapActivation()
{
	if ( autoMode)
	{
	NetMessage msg;

		msg.Reset( NetMessage::CSwap);
		msg.AddVariable( UnsignedToString( (unsigned)state.localHand[0], 2));
		state.localHand.Remove( state.localHand[0]);
		conn.SendMessage( msg);
	}
	else
	{
		task = GetSwap;
		iface->AddEvent( NotificationEvent( "Select a card to give the scumbag."));
		iface->SetMode( Interface::InGameSwapping);
	}
}