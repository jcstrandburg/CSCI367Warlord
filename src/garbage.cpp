/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

garbage.cpp

A quick and dirty testing utility that essentially just opens a connection to
a server on localhost and pours garbage onto the connection until the connection
is closed. This should be an effective test of the robustness of the server
=============================*/

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "netmanager.h"
#include "connection.h"
#include "netmessage.h"
#include "input.h"

void Pause( int millis)
{
#ifdef WIN32
	Sleep( millis);
#else
	usleep( millis*1000);
#endif
}

class TokenHolder {
	vector<string> tok;
public:
	TokenHolder() {}
	void Add( const string &s) { tok.push_back( s); }
	const void Get( string &s) {

		int mode = rand()%10;

		if ( mode  < 2) {
		char buf[20];

			sprintf( buf, "%2d", rand()%100);
			s = buf;
		}
		else if ( mode < 3 ) {
		char buf[2] = " ";

			buf[1] = (char)(rand()%255);
			s = buf;
		}
		else {
		int size = tok.size()*tok.size();
		int index = (int)sqrt( rand()%size);

				s = tok[ index];
		}
	}

};


int main( int argc, char *argv[]) 
{
NetManager net;
Connection conn;
TokenHolder tokens;

	srand( time(0));

	InitConsole();

	tokens.Add( "[");
	tokens.Add( "]");
	tokens.Add( "|");
	tokens.Add( ",");
	tokens.Add( ":");
	tokens.Add( ";");
	tokens.Add( "chand");
	tokens.Add( "cplay");
	tokens.Add( "cchat");
	tokens.Add( "Jon");
	tokens.Add( "Ned");
	tokens.Add( "Icarus");
	tokens.Add( "[cjoin|bbobbbob]");

	try {

		/*
		while ( true) {
			printf( "%s\n", tokens.Select().c_str());
		}*/

		while ( true) {

			int bytes = 0;
			string s;
			printf( "Opening a connection...\n");

			conn = net.RequestConnection( "localhost", 36738);

			while ( conn.IsOpen()) {

				tokens.Get( s);
				Pause( 0);
				conn.TransmitRawData( s.c_str(), s.size());
				bytes += s.size();
				net.Update();
			}

			printf( "Connection was closed after %d bytes\n", bytes);
		}
	}
	catch ( const char *c) {
		printf( "Exception: (%s)\n", c);
	}
	
	RevertConsole();
}
