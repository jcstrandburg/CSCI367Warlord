/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

server.cpp

Implementation for the server class, which does all the logic for the server application
=============================*/

#include <stdio.h>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

#include "warlord.h"
#include "server.h"
#include "netmanager.h"
#include "netmessage.h"
#include "wlconnection.h"
#include "input.h"
#include "wlclient.h"
#include "timer.h"
#include "card.h"
#include "serverplaystate.h"
#include "serverinterfaceadapter.h"

//helper function for formatting output through Trace and whatnot
//found this on the internet, didn't actually write it myself. Can't remember where found
std::string FormatString(const std::string fmt, ...) 
{
    int size = 100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
}

#define CONNECTION_QUEUE 30

using namespace std;

const unsigned int numSeats = 7;
const bool restrictTableUpdates = true;//if true, table updates are only sent when active player changes (or when active player needs to submit a new play)





//================================================
//PlayerState class definition
//================================================


PlayerState::PlayerState()
{
	status = Empty;
	client.reset( (WLClient*)null);
	currentRanking = nextRanking = -1;
}
string PlayerState::Serialize()
{
string s;

	//status, then strikes, then name, then number of cards
	switch ( status)
	{
	case Active:
		s += "a";
		break;
	case Passed:
		s += "p";
		break;
	case Waiting:
		s += "w";
		break;
	case Disconnected:
		s += "d";
		break;
	case Empty:
		s += "e";
		break;
	}

	if ( client)
	{
		s += UnsignedToString( client->GetStrikes(),1);
		s += ":";
		s += PadString( client->GetIdentifier(), clientNameLength);
		s += ":";
		s += UnsignedToString( hand.Size(), 2);
	}
	else
	{
		s += "0:xxxxxxxx:00";
	}

	return s;
}

//helper function for sorting
int PlayerState::Ordering() const
{
	if ( status == Empty)
		return 20000;
	else if ( status == Disconnected)
		return 10000;
	else
		return currentRanking;
}

bool PlayerState::operator < (const PlayerState &p) const
{
	return this->Ordering() < p.Ordering();
}


//================================================
//PlayState class definition
//================================================


PlayState::PlayState()
{
	Reset();
}

void PlayState::Reset()
{
	swapCard = NoCard;
	playersFinished = numPlayers = gameCounter = activePlayer = 0;
	lastPlay.Clear();
	firstTurn = true;
	updated = true;
	d = false;
	players.clear();
	players.resize( 7, PlayerState());
	status = NoGame;
}

PlayerState* PlayState::FindPlayerByName( const string &name)
{
unsigned int i;

	for ( i = 0; i < numPlayers; i++)
	{
		if ( players[i].client->GetIdentifier() == name )
		{
			return &players[i];
		}
	}
	return null;
}

bool PlayState::IsWarlord( const string &name)
{
PlayerState *p = FindPlayerByName( name);

	if ( p != null)
	{
		return p->currentRanking == 0;
	}
	return false;
}




//================================================
//Server class definition
//================================================

//constructor
Server::Server( InterfacePtr ip, int prt, int tout, int lobbTime, int minPlr, bool log):
		iface( ip), listenPort( prt), playTimeout( tout), lobbyTimeout( lobbTime), minPlayers( minPlr)
{
	IfaceAdapterPtr p( new ServerInterfaceAdapter( &state));
	iface->SetInterfaceAdapter( p);

	exitCondition = false;
	lobbySize = 0;
	chatThrottleTimer.Restart();

	if ( log)
		net.StartLogging( "slogs");
}

Server::~Server()
{
	
}

//debugging function, get rid of this later
void Server::EchoTableStatus()
{
unsigned int i;
NetMessage m;
string s;

	BuildTableStatusMsg( m);
	iface->Trace( FormatString( "?:%s", m.Serialize().c_str()));

	s = "Game state: ";
	switch ( state.status)
	{
	case PlayState::NoGame:
		s += "no game";
		break;
	case PlayState::EnteringGame:
		s +=  "entering game";
		break;
	case PlayState::WaitingForSwap:
		s +=  "waiting for swap";
		break;
	case PlayState::WaitingForPlay:
		s +=  "waiting for play";
		break;
	}
	iface->Trace( s);

	unsigned int count = 0;

		for ( i = 0; i < numSeats; i++)
		{
			if ( state.players[i].status != PlayerState::Disconnected &&
				 state.players[i].status != PlayerState::Empty &&
				 state.players[i].hand.Size() != 0 )
			{
				count++;
			}
		}


	iface->Trace( FormatString( "Viable players - %d", count));


	iface->Trace( FormatString( "%d players seated", state.numPlayers));
	for ( i = 0; i < numSeats; i++)
	{
		s = FormatString( "\tSeat %d: ", i);
		if ( state.players[i].status == PlayerState::Empty)
			s += FormatString( "empty with %u cards\n", state.players[i].hand.Size());
		else
		{
			s += FormatString( "%s(%d) with %u cards|", state.players[i].client->GetIdentifier().c_str(), state.players[i].client->GetStrikes(), state.players[i].hand.Size());
			if ( state.players[i].client->connection.IsOpen())
			{
				s += "open";
			}
			else
			{
				s += "closed";
			}
		}
		iface->Trace( s);
	}
}

//returns true if we have received a quit signal from the interface
bool Server::ExitCondition()
{
	return iface->QuitReceived();
}

void Server::StartUp()
{
	iface->Trace( FormatString( "Listening for connections on port %d", listenPort));
	net.Listen( listenPort, CONNECTION_QUEUE);
}


//doi
void	Server::Update()
{
ClientIterator iter;
NetMessage msg;

	net.Update();
	lobbyChanged = false;
	CheckForConnections();
	CheckForDisconnects();
	ProcessMessages();

	if ( chatThrottleTimer.GetMsec() > 1000 )
	{
		chatThrottleTimer.Restart();
		DecrementChatThrottles();
	}

	if ( state.status == PlayState::NoGame && lobbySize >= minPlayers )
	{
		iface->Trace("Starting lobby timer");
		state.timeout.Restart();
		state.status = PlayState::EnteringGame;
	}
	else if ( state.status == PlayState::EnteringGame)
	{
		if ( lobbySize >= minPlayers )
		{
			if ( state.timeout.GetMsec() > lobbyTimeout)
			{
				DoPreGame();
			}
		}
		else
		{
			iface->Trace( "Lost players, going back to default state");
			state.status = PlayState::NoGame;
		}
	}
	else if ( state.status == PlayState::WaitingForSwap)
	{
		//if the players time has run out give them a strike and cancel the swap
		if ( state.timeout.GetMsec() > playTimeout)
		{
			GiveStrike( state.players[0].client, Errors::Timeout);
			CancelSwap();
		}
	}
	else if  ( state.status == PlayState::WaitingForPlay )
	{
		//if the players time has runout give them a strike and force them to pass
		if ( state.timeout.GetMsec() > playTimeout )
		{
			GiveStrike( state.players[ state.activePlayer].client, Errors::Timeout);
			DoPlay( &state.players[ state.activePlayer], CardList());
			state.timeout.Restart();
		}

		//check the end game condition
		if ( CheckGameOver() == true )
		{
			DoPreGame();
		}
	}

	//check to see if we need to send out a lobby status message
	if ( lobbyChanged)
	{
		BuildLobbyStatusMsg( msg);
		BroadcastMessage( msg);
	}

	//check to see if we need to send out a table status message
	if ( state.updated == true && state.status == PlayState::WaitingForPlay )
	{
		state.updated = false;

		BuildTableStatusMsg( msg);
		BroadcastMessage( msg);
		iface->AddEvent( InterfaceEvent::TableUpdate, "", CardList(), "");
	}
}

//check for incoming messages from all clients and process them appropriately
void 	Server::ProcessMessages()
{
ClientIterator iter;
WLConnection::MessageStatus status;
NetMessage msg;

	for ( iter = clients.begin(); iter != clients.end(); iter++)
	{
		while ( (status = (*iter)->connection.GetMessageStatus()) != WLConnection::NoMessage )
		{
			switch (status)
			{
				case WLConnection::LengthExceeded:
					(*iter)->connection.ClearGarbage();
					iface->Trace(  "Message length exceeded");
					GiveStrike( *iter, Errors::UnknownError);
					break;
				case WLConnection::Malformed:
					(*iter)->connection.ClearGarbage();
					iface->Trace( "Malformed message");
					GiveStrike( *iter, Errors::BadMessage);
					break;
				case WLConnection::GoodMessage:
					(*iter)->connection.GetMessage( msg);
					switch ( msg.type)
					{
						case NetMessage::Invalid:
							GiveStrike( *iter, Errors::UnknownMsgType);
							break;
						case NetMessage::CJoin:
							HandleJoinMessage( *iter, msg);
							break;
						case NetMessage::CChat:
							HandleChatMessage( *iter, msg);
							break;
						case NetMessage::CSwap:
							HandleSwapMessage( *iter, msg);
							break;
						case NetMessage::CPlay:
							HandlePlayMessage( *iter, msg);
							break;
						case NetMessage::CHand:
							HandleCHandMessage( *iter, msg);
							break;
						default:
							iface->Trace( FormatString( "Unhandled message received: %s", msg.Serialize().c_str()));
							GiveStrike( *iter, Errors::UnknownError);
							break;
					}
					break;
				case WLConnection::NoMessage://needed for compiler warnings, should never get here
					break;
			}
		}
	}
}

//check for incoming connection requests and create client objects for all of them,
//if the client list exceeds a certain size the connection will be rejected
void	Server::CheckForConnections()
{
WLConnection conn;

	while ( net.HasConnectRequest())
	{
		conn = net.AcceptConnectRequest();

		if ( clients.size() < maxClients )
			clients.push_back( WLClientPtr( new WLClient( conn)));
		else
			RejectConnection( conn);
	}
}

//check for clients that have disconnected and react accordingly
void	Server::CheckForDisconnects()
{
ClientIterator iter;
ClientIterator iter2;

	for ( iter = clients.begin(); iter != clients.end(); iter++)
	{
		if ( !(*iter)->connection.IsOpen() && (*iter)->GetStatus() != WLClient::Disconnected)
		{
			switch ( (*iter)->GetStatus())
			{
			case WLClient::Uncertified://just remove this client from the list
				clients.erase( iter--);
				break;
			case WLClient::InLobby://update the lobby and whatnot
				lobbySize--;
				iface->AddEvent( InterfaceEvent::PlayerLeaveLobby, (*iter)->GetIdentifier(), CardList(), "");
				lobbyChanged = true;
				(*iter)->Disconnect();
				break;
			case WLClient::InGame://update the game state
				HandleInGameDisconnect( state.FindPlayerByName( (*iter)->GetIdentifier()));
				(*iter)->Disconnect();
				iface->Trace( FormatString( "A player (%s) in game has disconnected", (*iter)->GetIdentifier().c_str()));
				break;
			case WLClient::Disconnected://should never get here, but needed for compiler warning
				break;
			}
		}
	}

	if ( state.status == PlayState::NoGame || state.status == PlayState::EnteringGame )
		DeleteDisconnectedClients();
}

//helper function for removing clients from the the client list
bool ClientDisconnected( const WLClientPtr c)
{
	return !c->connection.IsOpen();
}

//does what it says
void	Server::DeleteDisconnectedClients()
{
	clients.remove_if( ClientDisconnected);
}

//decreases the chat throttle of all connected clients
void	Server::DecrementChatThrottles()
{
ClientIterator iter;

	for ( iter = clients.begin(); iter != clients.end(); iter++)
	{
		(*iter)->DecreaseChatThrottle();
	}
}




//returns true if some client in lobby or in game (or disconnected but the client object still exists) has the identifier s
bool Server::ClientNameConnected( string &s)
{
ClientIterator iter;

	for ( iter = clients.begin(); iter != clients.end(); iter++)
	{
		if ( (*iter)->GetIdentifier() == s )
			return true;
	}

	return false;
}

//Sends a strike message to the client, increments their strike count, and cuts their throat if
//they have too many strikes
void 	Server::GiveStrike( WLClientPtr c, Errors::ErrorCode err)
{
NetMessage msg;
string s;

	msg.Reset( NetMessage::STrik);
	s = UnsignedToString( err, 2);
	msg.AddVariable( s);
	if ( c->GetStrikes () < 3)
		c->TakeStrike();
	s = UnsignedToString( c->GetStrikes(), 1);
	msg.AddVariable( s);
	c->connection.SendMessage( msg);

	if ( c->GetStrikes() >= 3 )
	{
		c->connection.Close();
	}

	iface->Trace( FormatString( "Sending strike to client %s with error code %d", c->GetIdentifier().c_str(), err));
}

//Modifies the game state appropriately when a player in game disconnects for any reason
void	Server::HandleInGameDisconnect( PlayerState *p)
{
	if ( p != null )
	{
		//technically the table status has changed, but sending unnecessary messages can mess up client behaviour
		if ( !restrictTableUpdates)
			state.updated = true;

		//the the player is active make them inactive and then activate the next player
		if ( p->status == PlayerState::Active )
		{
			AdvancePlayerIndex();
			state.players[ state.activePlayer].status = PlayerState::Active;
			state.timeout.Restart();
			state.updated = true;
		}

		//check to see if the disconnected player was swapping
		if ( state.status == PlayState::WaitingForSwap && p == &state.players[0])
		{
			CancelSwap();
		}

		p->status = PlayerState::Disconnected;
		p->hand.Clear();
		state.playersFinished++;
	}
}

//Mangles the given identifier. If it has a number at the end of it the number will be incremented
//else a 1 will be appended to the name. 
string Server::Mangle( string &s)
{
string digits;//the trailing digits on the end of the name
string alphas;//everything before the trailing digits
int number = 0;
int index = s.size();
char buffer[clientNameLength];

	while ( isdigit( s[index-1]))
		index--;

	alphas = s.substr( 0, index);
	digits = s.substr( index, s.size()-index+1);

	if ( digits.size() > 0 )
	{
		sscanf( digits.c_str(), "%d", &number);
	}
	sprintf( buffer, "%d", number+1);
	digits = buffer;

	return alphas.substr( 0, clientNameLength-digits.size()) + digits;
}

//remove invalid characters and padding from a client identifier
string Server::PreMangle( string &s)
{
string out;
bool started = false;
unsigned int i;

	out.reserve( clientNameLength);
	for ( i=0; i < s.size() && out.size() < clientNameLength; i++)
	{
		if ( isalpha( s[i]) || s[i] == '_' )
		{
			started = true;
			out.push_back( s[i]);
		}
		else if ( isdigit( s[i]) and started)
		{
			out.push_back( s[i]);
		}
	}

	if ( out.size() < 1 )
		out = "Dummy";

	return out;
}

//sends the given connection a server full message then closes it
void	Server::RejectConnection( WLConnection c)
{
NetMessage msg;

	msg.Reset( NetMessage::STrik);
	msg.AddVariable( UnsignedToString( 3, 2));
	msg.AddVariable( UnsignedToString( Errors::ServerFull, 2));
	c.SendMessage( msg);
	c.Close();
}

//mangles the given client identifier until it is not claimed by another client
string Server::ValidateName( string &s)
{
string out;
string out2;

	out = PreMangle( s);
	while ( ClientNameConnected( out) )
	{
		out = Mangle( out);
	}
	return out;
}





//advances the activePlayer index
void	Server::AdvancePlayerIndex()
{
unsigned orig = state.activePlayer;

	//if there is only one viable player left in the game then this method needs to just return
	if ( CheckGameOver())
		return;

	//advance at least once, keep advancing until we find a suitable player or wrap back around to where we started
	do
	{
		state.activePlayer = (state.activePlayer+1)%state.numPlayers;

		//first check for disconnects
		if ( state.players[state.activePlayer].status == PlayerState::Disconnected)
		{
			//do nothing
		}
		//now check for players that are out
		else if ( state.players[state.activePlayer].hand.Size() == 0)
		{
			//state.players[state.activePlayer].status = PlayerState::Passed;
		}
		else
		{
			return;
		}
	} while ( state.activePlayer != orig);

	//this is bad
	throw "AdvancePlayerIndex wrapped around!";
}


//this method does everything necessary to cancel swap task, reseting state appropriately and starting
//the game and whatnot
void Server::CancelSwap()
{
	DoCardSwap( 52, 52);
	StartGame();
}

//checks to see if the game is over, returning true if so
bool Server::CheckGameOver()
{
unsigned int i;
unsigned int count = 0;

	for ( i = 0; i < numSeats; i++)
	{
		if ( state.players[i].status != PlayerState::Disconnected &&
			 state.players[i].status != PlayerState::Empty && 				
			 state.players[i].hand.Size() != 0 )
		{
			++count;
		}
	}

	return count <= 1;
}

//deals out a full 52 card deck to the given vector of CardLists
void	Server::DealHands( vector<CardList> &hands)
{
Card deck[52], temp;
unsigned int i;//loop iterator
unsigned int cardCount;//number of cards remaining in "deck"
unsigned int swapRand;//index of random card to be swapped in shuffling process
unsigned int currentHand = 0;//index of hand receiving next delt card

	for ( i = 0; i < hands.size(); i++)
		hands[i].Clear();

	//create the deck
	for ( i = 0; i < 52; i++)
		deck[i] = i;
	
	srand( time(0));
	cardCount = 52;
	while ( cardCount > 0)
	{
		cardCount--;
		if ( cardCount > 0)
		{
			//select a random index for swapRand, then swap the last card in deck with the randomly selected card
			swapRand = rand()%(cardCount+1);
			temp = deck[swapRand];
			deck[swapRand] = deck[cardCount];
			deck[cardCount] = temp;
		}

		//deal out the last card in deck to the current hand, then rotate the current hand index
		hands[ currentHand].Add( deck[cardCount]);
		currentHand = (currentHand + 1)%hands.size();
	}
}

//do the beginning of game card swap, giving wlCard to the scumbag and sbCard to the warlord
//handles sending out the message to the scumbag
void 	Server::DoCardSwap( Card wlCard, Card sbCard)
{
NetMessage msg;
CardList *hand1, *hand2;

	if ( wlCard != NoCard && sbCard != NoCard )
	{
		hand1 = &state.players[0].hand;
		hand2 = &state.players[ state.numPlayers-1].hand;

		hand1->Remove( wlCard);
		hand1->Add( sbCard);

		//make sure the scumbag didnt disconnect while waiting for the warlord to swap
		if ( state.players[ state.numPlayers-1].status != PlayerState::Disconnected)
		{
			hand2->Remove( sbCard);
			hand2->Add( wlCard);

			//inform the scumbag about the swappery
			msg.Reset( NetMessage::SWaps);
			msg.AddVariable( UnsignedToString( (unsigned)wlCard, 2));
			msg.AddVariable( UnsignedToString( (unsigned)sbCard, 2));
			state.players[ state.numPlayers-1].client->connection.SendMessage( msg);
		}

		iface->Trace( FormatString( "The warlord swapped his %s for the scumbags %s", wlCard.CardToString().c_str(), sbCard.CardToString().c_str()));
	}
}


//carries out the given play. It is assumed that the given play is valid (as defined by ValidPlay)
void	Server::DoPlay( PlayerState *p, const CardList &cards)
{
unsigned int i, waitingCount;
string s;

	//if its not a pass, do stuff
	if ( cards.Size() > 0 )
	{
		s = FormatString( "Player %s played {", p->client->GetIdentifier().c_str());
		for ( i = 0; i < cards.Size(); i++)
		{
			p->hand.Remove( cards[i]);
			s += FormatString( " %s", cards[i].CardToString().c_str());
		}
		p->status = PlayerState::Waiting;
		s += "}";
		iface->Trace( s);

		//check to see if we need to skip
		if ( (state.lastPlay.Size() > 0) && (cards[0].GetFaceValue() == state.lastPlay[0].GetFaceValue())
			&& state.lastPlay.Size() == cards.Size() )
		{
			//skip a player
			iface->Trace( "Skipping a player");
			AdvancePlayerIndex();
			state.players[ state.activePlayer].status = PlayerState::Passed;
		}
		state.lastPlay = cards;
		
		//check to see if the player went out
		if ( p->hand.Size() == 0)
		{
			p->nextRanking = state.playersFinished++;
			iface->Trace( FormatString( "Player %s has gone out", p->client->GetIdentifier().c_str()));
		}
	}
	//its a pass
	else
	{
		iface->Trace( FormatString( "Player %s passed", p->client->GetIdentifier().c_str()));
		p->status = PlayerState::Passed;
	}

	if ( state.lastPlay.Size() > 0 && state.lastPlay[0].GetFaceValue() == Card::Face2)
	{
		state.lastPlay.Clear();
	}
	else
	{
		AdvancePlayerIndex();
	}
	state.players[ state.activePlayer].status = PlayerState::Active;
	
	//check for end round condition (if no players have waiting status)
	waitingCount = 0;
	for ( i = 0; i < state.numPlayers; i++)
	{
		if ( state.players[i].status == PlayerState::Waiting && state.players[i].hand.Size() > 0 )
			waitingCount++;
	}
	if ( waitingCount == 0 )
	{
		iface->Trace( "Clearing last play.");
		state.lastPlay.Clear();
	}

	state.timeout.Restart();
	state.firstTurn = false;
	state.updated = true;
}

//Deal the hands, prep the game state, and either initiate a swap or start the
//game as appropriate
void 	Server::DoPreGame()
{
NetMessage msg;
ClientIterator iter;
unsigned int i;

	state.gameCounter++;
	state.updated = true;
	state.playersFinished = state.activePlayer = 0;
	state.lastPlay.Clear();
	state.firstTurn = true;

	//kick disconnected players from the table
	for ( i = 0; i < numSeats; i++)
	{
		if ( state.players[i].status != PlayerState::Empty )
		{
			//if the player is disconnected (or needs to be)
			if ( state.players[i].client == null || state.players[i].client->GetStatus() == WLClient::Disconnected )
			{
				state.players[i].status = PlayerState::Empty;
				state.players[i].hand.Clear();
				state.players[i].client.reset( (WLClient*)null);
				state.numPlayers--;
			}
			//else the player is still in game
			{
				state.players[i].currentRanking = state.players[i].nextRanking;
			}
		}
	}

	//now that disconnected clients are guaranteed to not be at the table we
	//can delete them
	DeleteDisconnectedClients();


	//sort players by ranking
	std::sort( state.players.begin(), state.players.end());

	//grab new players from the lobby
	for ( iter = clients.begin(); iter != clients.end() && state.numPlayers < numSeats; iter++)
	{
		if ( (*iter)->GetStatus() == WLClient::InLobby )
		{
			SeatClient( *iter);
		}		
	}
	
	//check to make sure we have enough players, if not go back to lobby
	if ( state.numPlayers < minPlayers )
	{
		iface->Trace( "Not enough players. Going back to lobby!");
		state.status = PlayState::NoGame;
		for ( i = 0; i < state.numPlayers; i++)
		{
			if ( state.players[i].client->GetStatus() == WLClient::InGame )
			{
				state.players[i].client->ReturnToLobby();
				iface->AddEvent( InterfaceEvent::PlayerEnterLobby, state.players[i].client->GetIdentifier(), CardList(), "");
				lobbySize++;
			}
		}
		state.Reset();
		DeleteDisconnectedClients();
		lobbyChanged = true;
		return;
	}

	//deal out the hands now that we know how many we need, also set next ranking to something high
	//do some other set up too
	if ( state.numPlayers > 0 )
	{
	vector<CardList> hands( state.numPlayers, CardList());
	unsigned int i;

		DealHands( hands);
		for ( i = 0; i < state.numPlayers; i++)
		{
			state.players[i].hand = hands[i];
			state.players[i].status = PlayerState::Waiting;
			state.players[i].currentRanking = i;
			state.players[i].nextRanking = 1000;
		}
	}

	//if first round, arrange the table so the player with 3 of clubs starts
	if ( state.gameCounter ==  1)
	{
	unsigned int startPlayer;

		//set the state properly, and find the player with the 3 of clubs
		state.status = PlayState::WaitingForPlay;
		for ( startPlayer = 0; startPlayer < state.numPlayers; startPlayer++)
		{
			if ( state.players[startPlayer].hand.Contains( Card( Card::SuitClubs, Card::Face3)))
			{
				break;
			}
		}

		//adjust the rankings of all the players then run a sort, the player with C3 should be first now
		if ( startPlayer < state.numPlayers)
		{
		unsigned int i;
			for ( i = 0; i < state.numPlayers; i++)
			{
				state.players[i].currentRanking = (state.players[i].currentRanking+(state.numPlayers-startPlayer))%state.numPlayers;
			}
			std::sort( state.players.begin(), state.players.end());
		}
		else
		{
			throw "Could not find 3 of clubs. Madness I say!";
		}

		StartGame();
	}
	//else start the swap process
	else
	{
		//grab the highest card from the scumbag and save it
		state.swapCard = state.players[ state.numPlayers-1].hand.HighestCard();

		//send the warlord his original hand
		BuildHandMsg( msg, state.players[0].hand);
		state.players[0].client->connection.SendMessage( msg);
		
		//send the pilfered card to the warlord
		msg.Reset( NetMessage::SWapw);
		msg.AddVariable( UnsignedToString( (unsigned)state.swapCard, 2));
		state.players[0].client->connection.SendMessage( msg);

		//waiting for a swap message back from the warlord, time to skiddadle so we don't block
		state.status = PlayState::WaitingForSwap;
		state.timeout.Restart();
	}
}

//returns true if a game is currently being played
bool	Server::InGame()
{
	return state.gameCounter > 0;
}


//assigns the given client a seat in the game, updating the clients status
//and assigning the player a ranking
void Server::SeatClient( WLClientPtr c)
{
unsigned int i;

	assert( c->GetStatus() == WLClient::InLobby);

	if ( state.numPlayers >= numSeats )
		return;

	c->EnterGame();
	iface->AddEvent( InterfaceEvent::PlayerLeaveLobby, c->GetIdentifier(), CardList(), "");

	lobbySize--;
	lobbyChanged = true;
	state.d = true;
	i = state.numPlayers;
	state.players[i].status = PlayerState::Waiting;
	state.players[i].currentRanking = i;
	state.players[i].nextRanking = i;
	state.players[i].hand.Clear();
	state.players[i].client = c;
	state.numPlayers++;
}

//does what it says
void	Server::StartGame()
{
NetMessage msg;
unsigned int i;

	iface->Trace( FormatString( "Starting game %d", state.gameCounter));

	//send out the hand messages
	for ( i = 0; i < state.numPlayers; i++)
	{
		BuildHandMsg( msg, state.players[i].hand);
		state.players[i].client->connection.SendMessage( msg);
	}
	
	//find the first player who isn't disconnected and make them active
	for ( i = 0; i < state.numPlayers; i++)
	{
		if ( state.players[i].client && state.players[i].client->connection.IsOpen() )
		{
			state.players[i].status = PlayerState::Active;
			break;
		}
	}

	state.timeout.Restart();
	state.status = PlayState::WaitingForPlay;
}



//Checks the given cards to see if they are a valid play. The only precondition for this method
//is that the elements of cards are valid cards. It is the responsibility of this method
//to verify that the play is legal and the player has the cards etc.
//this method will give out strikes and modify game state!
bool 	Server::ValidPlay( PlayerState *p, const CardList &cards)
{
unsigned int i;
	
	//make sure the player is in game and active
	if ( p == null)
	{
		return false;
	}
	else if ( p->status != PlayerState::Active)
	{
		iface->Trace( FormatString( "WLClient (%s) trying to play out of turn", p->client->GetIdentifier().c_str()));
		GiveStrike( p->client, Errors::OutOfTurn);
		return false;
	}
	
	//check to see if its a pass
	if ( cards.Size() == 0 )
	{
		if ( state.firstTurn)
		{
			iface->Trace( FormatString( "Player %s attempting to pass on first turn", p->client->GetIdentifier().c_str()));
			GiveStrike( p->client, Errors::PassOnStart);
			return false;
		}
		else
		{
			return true;
		}
	}
	
	//make sure all cards have same face value
	for ( i = 1; i < cards.Size(); i++)
	{
		if ( cards[i].GetFaceValue() != cards[0].GetFaceValue())
		{
			iface->Trace( FormatString( "Player %s attempted to play cards of differing faces", p->client->GetIdentifier().c_str()));
			GiveStrike( p->client, Errors::DifferingFaces);
			return false;
		}
	}
	
	//make sure the player has all the cards he is trying to play
	for ( i = 0; i < cards.Size(); i++)
	{
		if ( !p->hand.Contains( cards[i]))
		{
			iface->Trace( FormatString( "Player %s attempted to play card he does not have", p->client->GetIdentifier().c_str()));
			GiveStrike( p->client, Errors::CardNotInHand);
			return false;
		}
	}
	
	//make sure they are playing the 3 of clubs if they are obligated to
	if ( state.gameCounter == 1 && state.firstTurn == true &&
		!cards.Contains( Card( Card::Face3 + Card::SuitClubs))
		&& p->hand.Contains( Card( Card::Face3 + Card::SuitClubs)))
	{
		iface->Trace( "Invalid first play!");
		GiveStrike( p->client, Errors::InvalidFirstTurn);
		SendHandMsg( p);
		return false;
	}

	//make sure the cards are high enough
	if ( state.lastPlay.Size() > 0 && (state.lastPlay[0].GetFaceValue() > cards[0].GetFaceValue()) )
	{
		iface->Trace( "Player tried to play cards that are not high enough");
		GiveStrike( p->client, Errors::LowFaceValue );
		return false;
	}

	//make sure there are enough cards
	if ( (state.lastPlay.Size() > cards.Size()) && cards[0].GetFaceValue() != Card::Face2)
	{
		iface->Trace( FormatString( "Player %s did not play enough cards", p->client->GetIdentifier().c_str()));
		GiveStrike( p->client, Errors::LowCardCount);
		return false;
	}
	
	//if we got here everything is good
	return true;
}


//sends the given message to all clients that are not disconnected
void	Server::BroadcastMessage( NetMessage &msg)
{
ClientIterator iter;

	for ( iter = clients.begin(); iter != clients.end(); iter++)
	{
		if ( (*iter)->GetStatus() != WLClient::Disconnected)
			(*iter)->connection.SendMessage( msg);
	}
}

//sends the given string s as a chat message to all clients, including the original sender
void	Server::SendChat( const string &name, const string &s)
{
ClientIterator iter;
NetMessage msg;

	msg.Reset( NetMessage::SChat);
	msg.AddVariable( name);
	msg.AddVariable( s);

	BroadcastMessage( msg);
}

void	Server::SendHandMsg( PlayerState *p)
{
NetMessage msg;

	if ( p)
	{
		BuildHandMsg( msg, p->hand);
		p->client->connection.SendMessage( msg);
	}
}

void	Server::SendTableMsg( PlayerState *p)
{
NetMessage msg;

	if ( p && p->client )
	{
		BuildTableStatusMsg( msg);
		p->client->connection.SendMessage( msg);
	}
}






void	Server::HandleJoinMessage( WLClientPtr c, NetMessage &msg)
{
NetMessage msg2;	
string name;

	assert( c);

	if ( msg.vars.size() != 1)
	{
		GiveStrike( c, Errors::MalformedMessage);
		return;
	}
	if ( c->GetStatus() != WLClient::Uncertified)
	{
		GiveStrike( c, Errors::BadMessage);
		return;
	}
	
	name = ValidateName( msg.vars[0]);
	c->EnterLobby( name);
	iface->Trace( FormatString( "Player %s has joined the server", c->GetIdentifier().c_str()));
	iface->AddEvent( InterfaceEvent::PlayerEnterLobby, c->GetIdentifier(), CardList(), "");
	lobbyChanged = true;
	lobbySize++;
	msg2.Reset( NetMessage::SJoin);
	msg2.AddVariable( PadString( name, clientNameLength));
	c->connection.SendMessage( msg2);
}

void	Server::HandleChatMessage( WLClientPtr c, NetMessage &msg)
{
	assert( c);
	
	if ( msg.vars.size() != 1)
	{
		GiveStrike( c, Errors::MalformedMessage);
		return;
	}
	else if ( msg.vars[0].size() > 63)
	{
		GiveStrike( c, Errors::MalformedMessage);
		return;
	}
	else if ( c->GetStatus() == WLClient::Uncertified)
	{
		GiveStrike( c, Errors::BadMessage);
		return;
	}
	else if ( c->GetChatThrottle() >= chatLimit)
	{
		GiveStrike( c, Errors::ChatFlood);
		return;
	}

	SendChat( c->GetIdentifier(), PadString( msg.vars[0], 63));
	c->IncreaseChatThrottle();
}

//parses the message for card values, builds a list of cards (not including non cards for passing and whatnot)
//then passes the cards off to ValidPlay and DoPlay. The only error checking this method does is for out of range card values,
//malformed messages, and duplicate card values
void 	Server::HandlePlayMessage( WLClientPtr c, NetMessage &msg)
{
unsigned int i;
int cardval;
CardList cards;
vector<string> cardTokens;
PlayerState *p;
	
	assert( c);

	if ( msg.vars.size() != 1 )
	{
		iface->Trace( FormatString( "Invalid cplay message from %s", c->GetIdentifier().c_str()));
		GiveStrike( c, Errors::MalformedMessage);
		return;
	}
	
	ExplodeString( ",", msg.vars[0], cardTokens);
	if ( cardTokens.size() != 4 )
	{
		iface->Trace( FormatString( "Received play message from %s with an invalid number of cards", c->GetIdentifier().c_str()));
		GiveStrike( c, Errors::MalformedMessage);
		return;
	}
		
	for ( i = 0; i < cardTokens.size(); i++ )
	{
		int x =	sscanf( cardTokens[i].c_str(), "%d", &cardval);

		if ( x < 1 )
		{
			iface->Trace( FormatString( "Player %s sent a malformed cplay message", c->GetIdentifier().c_str(), cardval));
			GiveStrike( c, Errors::MalformedMessage);
			return;
		}
		else if ( cardval < 0 || cardval > 52)
		{
			iface->Trace( FormatString( "Player %s tried to play an out of range card (%d)", c->GetIdentifier().c_str(), cardval));
			GiveStrike( c, Errors::MalformedMessage);
			return;
		}
		else if ( cardval != 52)
		{
			if ( cards.Contains( Card( cardval)))
			{
				iface->Trace( FormatString( "Player %s tried to play duplicate cards", c->GetIdentifier().c_str()));
				GiveStrike( c, Errors::DuplicatedCard);
				return;
			}
			else
			{
				cards.Add( Card( cardval));
			}
		}
	}

	p = state.FindPlayerByName( c->GetIdentifier());
	if ( p)
	{
		if ( ValidPlay( p, cards))
		{
			DoPlay( p, cards);
		}
		else
		{
			//send just this player their hand and a table status message
			SendHandMsg( p);
			SendTableMsg( p);
		}
	}
	else
	{
		iface->Trace( FormatString( "WLClient (%s) not in game trying to play", c->GetIdentifier().c_str()));
		GiveStrike( c, Errors::NotInGame);		
	}
}

void	Server::HandleSwapMessage( WLClientPtr c, NetMessage &msg)
{
	assert( c);
	
	if ( !state.IsWarlord( c->GetIdentifier()))
	{
		iface->Trace( FormatString( "Non warlord player %s attempting to swap", c->GetIdentifier().c_str()));
		GiveStrike( c, Errors::UnknownError);
	}
	else if ( msg.vars.size() != 1)
	{
		iface->Trace( FormatString( "Rejecting swap message from %s with too many vars", c->GetIdentifier().c_str()));
		GiveStrike( c, Errors::MalformedMessage);
	}
	else
	{
	unsigned cardVal = 1000;
	int x;

		iface->Trace( FormatString( "%s", msg.Serialize().c_str()));

		x = sscanf( msg.vars[0].c_str(), "%u", &cardVal);
		if ( x < 1)
			GiveStrike( c, Errors::MalformedMessage);
		else if ( cardVal < 52 )
			DoCardSwap( Card( cardVal), state.swapCard);
		else
			GiveStrike( c, Errors::MalformedMessage);
		StartGame();
	}
}

void	Server::HandleCHandMessage( WLClientPtr c, NetMessage &msg)
{
	assert( c);
	
PlayerState *p = state.FindPlayerByName( c->GetIdentifier());

	if ( p)
	{
		iface->Trace( FormatString( "Player %s requesting hand!", c->GetIdentifier().c_str()));
		SendHandMsg( p);
	}
	else
	{
		GiveStrike( c, Errors::NotInGame);
	}
}



void	Server::BuildLobbyStatusMsg( NetMessage &msg)
{
ClientIterator iter;	
string s;
int x = 0;
	
	msg.Reset( NetMessage::SLobb);

	for ( iter = clients.begin(); iter != clients.end(); iter++)
	{
		if ( (*iter)->GetStatus() == WLClient::InLobby)
		{
			if ( s.size() > 0 )
				s += ",";
			s += PadString( (*iter)->GetIdentifier(), clientNameLength);
			x++;
		}
	}
	msg.AddVariable( UnsignedToString( x, 2));
	if ( x > 0)
		msg.AddVariable( s);
}

void	Server::BuildTableStatusMsg( NetMessage &msg)
{
unsigned int i;
string s;

	msg.Reset( NetMessage::STabl);
	
	//the player list
	s = "";
	for ( i = 0; i < numSeats; i++)
	{
		if ( i != 0 )
			s += ",";
		s += state.players[i].Serialize();
	}
	msg.AddVariable( s);
	
	//the last play
	s = "";
	for ( i = 0; i < state.lastPlay.Size(); i++)
	{
		if ( i != 0)
			s += ",";
		s += UnsignedToString( (unsigned)state.lastPlay[i], 2);
	}
	for ( ; i < 4; i++)
	{
		if ( i != 0)
			s += ",";		
		s += UnsignedToString( (unsigned)NoCard, 2);
	}
	
	
	msg.AddVariable( s);
	if ( state.gameCounter == 1)
		msg.AddVariable( UnsignedToString( 1, 1));
	else
		msg.AddVariable( UnsignedToString( 0, 1));
}

void	Server::BuildHandMsg( NetMessage &msg, const CardList &c)
{
unsigned int i;
string s;

	msg.Reset( NetMessage::SHand);
	s = "";
	for ( i = 0; i < c.cards.size(); i++)
	{
		if ( s.size() > 0)
			s += ",";
		s += UnsignedToString( (unsigned)(c.cards[i]), 2);
	}
	for ( ; i < maxHandSize; i++)
	{
		if ( s.size() > 0)
			s += ",";
		s += UnsignedToString( (unsigned)(NoCard), 2);
	}
	msg.AddVariable( s);
}