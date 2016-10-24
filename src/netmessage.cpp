/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

netmessage.h

Implementation for NetMessage module, which handles parsing of strings into a
NetMessage data structure
=============================*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netmessage.h"

#define stricmp strcasecmp

//binary comparator function for doing case insensitive comparisons on std::string objects, used in mapping strings to numerical values for message types
struct ci_less : binary_function<string, string, bool>
{
	bool operator() (const string &s1, const string &s2) const 
	{
		return stricmp( s1.c_str(), s2.c_str()) < 0;
	}
};

void ExplodeString( const string &delim, const string &s, vector<string> &out)
{
size_t pos = 0;
size_t start =  0;
string s2;

	out.clear();
	while ( (pos = s.find_first_of( delim, start)) != string::npos )
	{
		if ( pos != start )
		{
			out.push_back( RemovePadding( s.substr( start, pos-start)));
		}
		start = pos+1;
	}
	if ( start < s.size())
	{
		out.push_back( RemovePadding( s.substr( start, s.size()-start)));
	}
}

string RemovePadding( const string &s)
{
int first = 0;
int len = s.size();
	
	while( s[first] == ' ' && first < len )
		first++;
	while ( len > 0 && s[len-1] == ' ' )
		len--;
	
	len -= first;

	return s.substr( first, len);
}

//some formatting functions
string UnsignedToString( unsigned int x, unsigned int numDigits)
{
string s("");
char buffer[20];
unsigned int i;

	sprintf( buffer, "%u", x);
	for ( i = strlen( buffer); i < numDigits; i++)
		s += "0";
	s += buffer;
	return s;
}

string PadString( string s, unsigned int len)
{
	while ( s.size() < len)
		s += " ";
	return s;
}



NetMessage::NetMessage()
{
	type = Invalid;
}

NetMessage::NetMessage( const string &s)
{
	Parse( s);
}


void NetMessage::Reset( NetMessage::MsgType m)
{
	type = m;
	vars.clear();
}

NetMessage::MsgType NetMessage::StringToMsgType( const string &s) const
{
static map<string, MsgType> mappings;
map<string, MsgType>::iterator iter;

	if ( mappings.size() == 0 )
	{
		mappings["slobb"] = SLobb;
        mappings["stabl"] = STabl;
        mappings["sjoin"] = SJoin;
        mappings["shand"] = SHand;
        mappings["strik"] = STrik;
        mappings["schat"] = SChat;
        mappings["splay"] = SPlay;
        mappings["swaps"] = SWaps;
        mappings["swapw"] = SWapw;
        mappings["sswap"] = SSwap;
		mappings["clobb"] = CLobb;
        mappings["ctabl"] = CTabl;
        mappings["cjoin"] = CJoin;
        mappings["chand"] = CHand;
        mappings["ctrik"] = CTrik;
        mappings["cchat"] = CChat;
        mappings["cplay"] = CPlay;
        mappings["cwaps"] = CWaps;
        mappings["cwapw"] = CWapw;
        mappings["cswap"] = CSwap;		
	}
	
	iter = mappings.find( s);
	if ( iter == mappings.end() )
	{
		return Invalid;
	}
	
	return mappings[ s];
}

string NetMessage::MsgTypeToString( const MsgType m) const
{
static map<MsgType, string> mappings;

	if ( mappings.size() == 0 )
	{
		mappings[SLobb] = "slobb";
        mappings[STabl] = "stabl";
        mappings[SJoin] = "sjoin";
        mappings[SHand] = "shand";
        mappings[STrik] = "strik";
        mappings[SChat] = "schat";
        mappings[SPlay] = "splay";
        mappings[SWaps] = "swaps";
        mappings[SWapw] = "swapw";
        mappings[SSwap] = "sswap";
		mappings[CLobb] = "clobb";
        mappings[CTabl] = "ctabl";
        mappings[CJoin] = "cjoin";
        mappings[CHand] = "chand";
        mappings[CTrik] = "ctrik";
        mappings[CChat] = "cchat";
        mappings[CPlay] = "cplay";
        mappings[CWaps] = "cwaps";
        mappings[CWapw] = "cwapw";
        mappings[CSwap] = "cswap";
	}
	
	return mappings[ m];
}

string NetMessage::Serialize() const
{
string s("[");
unsigned int i;

	if ( type == Invalid)
	{
		return "[Invalid NetMessage]";
	}

	s += MsgTypeToString( type);
	
	for ( i = 0; i < vars.size(); i++)
	{
		s += "|" + vars[i];
	}
	
	return s + "]";
}

void NetMessage::AddVariable( const string &s)
{
	vars.push_back( s);
}

void NetMessage::Parse( const string &s)
{
vector<string> tokens;
vector<string>::iterator iter;
int i;

	ExplodeString( "[|]", s, tokens);
	
	if ( tokens.size() < 1 )
	{
		type = NetMessage::Invalid;
		vars.clear();
		return;
	}
	
	iter = tokens.begin();
	type = StringToMsgType( *iter);
	vars.resize( tokens.size()-1);
	i = 0;
	while ( (++iter) != tokens.end() )
	{
		vars[ i++] = *iter;
	}
}
