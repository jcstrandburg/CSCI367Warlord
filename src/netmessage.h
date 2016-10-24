/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

netmessage.h

Specification for NetMessage module, which handles parsing of strings
into a NetMessage data structure
=============================*/

#ifndef NETMESSAGE_H
#define NETMESSAGE_H

#include <string>
#include <map>
#include <vector>
#include <list>

#define MSG_LENGTH 1024

using namespace std;

typedef list<string> StringList;

string RemovePadding( const string &s);
void ExplodeString( const string &delim, const string &s, vector<string> &out);
string UnsignedToString( unsigned int x, unsigned int numDigits);
string PadString( string s, unsigned int len);

class NetMessage
{
public:
	enum MsgType
	{
		SLobb = 0,
        STabl,
        SJoin,
        SHand,
        STrik,
        SChat,
        SPlay,
        SWaps,
        SWapw,
        SSwap,
		CLobb,
        CTabl,
        CJoin,
        CHand,
        CTrik,
        CChat,
        CPlay,
        CSwap,
        CWaps,
        CWapw,
		NumTypes,
		Invalid,
	};

	NetMessage();
	NetMessage( const string &s);	
	void Reset( MsgType t);//change the type of this message and reset all the data
	MsgType StringToMsgType( const string &s) const;//convert a std::string to a MsgType enum
	string MsgTypeToString( const MsgType m) const;//convert a MsgType enum to std::string
	string Serialize() const;//convert the entire message into a string transmittable over a network
	void AddVariable( const string &s);//add another item to the message
	void Parse( const string &s);//read the string and make and make message from it

	MsgType type;
	vector<string> vars;
};

#endif
