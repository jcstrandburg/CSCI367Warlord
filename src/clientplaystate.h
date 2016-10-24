/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

clientplaystate.h

Specification for the structures used by the Client object to represent play state
=============================*/

#ifndef CLIENTPLAYSTATE_H_
#define CLIENTPLAYSTATE_H_

#include <vector>
#include <string>
#include <tr1/memory>
#include "card.h"

struct ClientPlayerState
{
enum Status
{
	Empty,
	Waiting,
	Active,
	Passed,
	Disconnected,
};
	
	string name;
	unsigned int handSize;
	Status status;
	unsigned int strikes;

	ClientPlayerState(): status( Empty) {}
};

struct ClientPlayState
{
vector<ClientPlayerState> players;
CardList lastPlay;
CardList localHand;//the hand of the player on the local host
string localName;//the name of the player on the local host
int numPlayers;
bool firstGame;

ClientPlayState(): players( 7, ClientPlayerState()), numPlayers( 0), firstGame( false) {}
};

typedef std::tr1::shared_ptr<ClientPlayState> ClientStatePtr;

#endif /*CLIENTPLAYSTATE_H_*/
