/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

card.h

Specification for card module, which deals with Card objects and CardList objects, for dealing with hands and plays and whatnot
=============================*/

#ifndef CARD_H
#define CARD_H

#include <vector>
#include <string>

using namespace std;

class Card
{
public:
enum FaceValue
{
	Face3=0,
	Face4=4,
	Face5=8,
	Face6=12,
	Face7=16,
	Face8=20,
	Face9=24,
	Face10=28,
	FaceJ=32,
	FaceQ=36,
	FaceK=40,
	FaceA=44,
	Face2=48,
	FaceNone=999,
};
	
enum Suit
{
	SuitClubs=0,
	SuitDiamonds=1,
	SuitHearts=2,
	SuitSpades=3,
	SuitNone=999,
};

	Card();
	Card( unsigned int val);
	Card( Suit s, FaceValue f);
	Card( const string& s);
	string FaceString() const;
	string SuitString() const;
	string CardToString() const;
	Suit GetSuit() const;
	FaceValue GetFaceValue() const;
	Card& operator = (const string &s);
	operator unsigned int () const;
private:
	unsigned int cardID;
};

class CardList
{
public:
void Add( Card c);	
bool Contains( Card c) const;
void Remove( Card c);
unsigned int Size() const;
unsigned int FindCard( Card c, int start, int end) const;
void Clear();
string Serialize() const;
Card HighestCard() const;
Card LowestCard() const;
const Card& operator [] ( const unsigned int index) const;

	vector<Card> cards;
};

extern const Card NoCard;

#endif /*CARD_H_*/
