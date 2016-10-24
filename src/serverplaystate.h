/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

serverplaystate.h

Specification for the structures used by the Server object to represent play state
=============================*/

#ifndef SERVERPLAYSTATE_H_
#define SERVERPLAYSTATE_H_

#include <tr1/memory>

#include "wlclient.h"
#include "timer.h"

typedef tr1::shared_ptr<WLClient> WLClientPtr;

struct PlayerState
{
enum State
{
	Active,
	Passed,
	Waiting,
	Disconnected,
	Empty,
};
	
CardList hand;
State status;
WLClientPtr client;
int currentRanking;//ranking goes from 0 to numPlayers
int nextRanking;

	PlayerState();
	string Serialize();	
	int Ordering() const;
	bool operator < (const PlayerState &p) const;
};


struct PlayState
{
enum GameState
{
	NoGame,
	EnteringGame,
	WaitingForPlay,
	WaitingForSwap,
};

vector<PlayerState> players;
unsigned int gameCounter;//which hand currently being played, 0 being no game started yet, 1 being the first hand
unsigned int numPlayers;//number of players currently seated at the table
bool d;//true if a table status message needs to be broadcast
Card swapCard;//the card that we took from the scumbag, need to keep track of it in case warlord drops
GameState status;//what GameState we are in
Timer timeout;//all purpose timeout timer
unsigned int playersFinished;//the number of players who have gone out this round
unsigned int activePlayer;//this index of the active player
CardList lastPlay;//the cards played last
bool firstTurn;
bool updated;

	PlayState();
	void Reset();
	PlayerState* FindPlayerByName( const WLClient::Identifier &name);
	bool IsWarlord( const WLClient::Identifier &name);
};

#endif /*SERVERPLAYSTATE_H_*/
