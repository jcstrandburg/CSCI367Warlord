/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

card.cpp

Implementation for card module, which deals with Card objects and CardList objects, for dealing with hands and plays and whatnot
=============================*/

#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>

#include "card.h"


const Card NoCard( 52);


Card::Card():
	cardID( 52)
{
}

Card::Card( unsigned int val):
	cardID( val)
{
	if ( cardID > 52 )
		cardID = 52;
}

Card::Card( Card::Suit s, Card::FaceValue f)
{
	if ( s == Card::SuitNone || f == Card::FaceNone )
		cardID = 52;
	else
		cardID = s+f;
}


Card::Card( const string& s)
{
	*this = s;
}

string Card::FaceString() const
{
	assert( cardID >= 0 && cardID <= 52);

	switch ( (cardID)/4)
	{
	case 0:
			return "3";
			break;
	case 1:
			return "4";
			break;
	case 2:
			return "5";
			break;
	case 3:
			return "6";
			break;
	case 4:
			return "7";
			break;
	case 5:
			return "8";
			break;
	case 6:
			return "9";
			break;
	case 7:
			return "10";
			break;
	case 8:
			return "J";
			break;
	case 9:
			return "Q";
			break;
	case 10:
			return "K";
			break;
	case 11:
			return "A";
			break;
	case 12:
			return "2";
			break;
	default:
			return "x";
			break;
	}
}

string Card::SuitString() const
{
	assert( cardID >= 0 && cardID <= 52);
	
	if ( cardID != 52)
	{	
		switch ( (cardID)%4)
		{
		case 0:
			return "C";
			break;
		case 1:
			return "D";
			break;
		case 2:
			return "H";
			break;
		case 3:
			return "S";
			break;
		}
	}
	return "x";
}

string Card::CardToString() const
{
	if ( cardID >= 0 && cardID < 52)
		return SuitString() + FaceString();
	else
		return "";
}

Card::Suit Card::GetSuit() const
{
	if ( cardID >= 0 && cardID < 52)		
		return Card::Suit(cardID%4);
	else
		return Card::SuitNone;		
}

Card::FaceValue Card::GetFaceValue() const
{
	if ( cardID >= 0 && cardID < 52)
		return Card::FaceValue(cardID - cardID%4);
	else
		return Card::FaceNone;
}

Card& Card::operator=(const string &s)
{
int suitModifier = 0, faceModifier = 52;
string suits( "CDHS");
string faces( "34567891JQKA2");
unsigned int i;
	
	if ( s.size() <= 3 )
	{
		for ( i=0; i < suits.size(); i++)
		{
			if ( suits[i] == s[0])
			{
				//printf( "%c == %c\n", suits[i], s[0]);
				suitModifier = i;
				break;
			}
			else
			{
				//printf( "%c != %c\n", suits[i], s[0]);				
			}
		}
		
		if ( i >= suits.size() )
		{
			*this = NoCard;
			return *this;
		}
		
		//if the string is longer than 2 and the last two characters aren't "10" this isnt a valid card and we need to skidaddle
		if ( s.size() > 2 and s.substr( 1, 2) != "10")
		{
			*this = NoCard;
			return *this;
		}
		
		
		for ( i=0; i < faces.size(); i++)
		{
			if ( faces[i] == s[1])
			{
				//printf( "%c == %c\n", faces[i], s[1]);				
				faceModifier = i*4;
				break;
			}
			else
			{
				//printf( "%c != %c\n", faces[i], s[1]);					
			}
		}
	}
	
	//printf( "%c => suitm (%d); %c => facem (%d) \n", s[0], suitModifier, s[1], faceModifier);
	cardID = faceModifier + suitModifier;
	return *this;
}

Card::operator unsigned int() const
{
	return cardID;
}





void CardList::Add( Card c)
{
vector<Card>::iterator iter;	
	
	if ( Contains(c))	
		return;

	iter = cards.begin();
	while ( iter != cards.end() && *iter < c )
		iter++;
	
	cards.insert( iter, c);
}

bool CardList::Contains( Card c) const
{
Card c2;
unsigned int i;
	for ( i=0; i < cards.size(); i++)
	{
		if ( cards[i] == c)
			return true;
	}
	return false;
}

void CardList::Remove( Card c)
{
int i;

	if ( cards.size() <= 0 )
		return;

	i = FindCard( c, 0, cards.size()-1);
	//make sure we actually found the card
	if ( i >= 0)
		cards.erase( cards.begin() + i);
}

unsigned int CardList::Size() const
{
	return cards.size();
}

string CardList::Serialize() const
{
string s("{");
unsigned int i;

	for ( i=0; i < cards.size(); i++)
	{
		s += cards[i].CardToString();
		if ( i != cards.size()-1)
			s+=", ";	
	}
	s+="}";

	return s;
}

void CardList::Clear()
{
	cards.clear();
}

unsigned int CardList::FindCard( Card c, int start, int end) const
{
unsigned int q;

	if ( start > end || start < 0 || end < 0)
		return -1;

	q = (start + end)/2;
	if ( cards[q] == c)
		return q;
	else if ( start >= end)
		return -1;
	else if ( cards[q] < c)
		return FindCard( c, q+1, end);
	else
		return FindCard( c, start, q-1);		
}

Card CardList::HighestCard() const
{
	return cards[ cards.size()-1];
}

Card CardList::LowestCard() const
{
	return cards[ 0];
}

const Card& CardList::operator [] ( const unsigned int index) const
{
	if ( index >= cards.size())
	{
	static char buffer[1024];
		sprintf( buffer, "Invalid Index in CardList::[%d]", index);
		throw buffer;
	}

	return cards[index];
}


