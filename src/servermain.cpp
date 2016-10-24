/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

servermain.cpp

Location of the main() function for the server application, which basically just reads command
line args and sets up the Server object then passes off responsibility
=============================*/

#include <stdio.h>
#include <list>
#include <string.h>
#include <stdlib.h>

#include "server.h"

#include "textinterface.h"
#include "cursesinterface.h"
#include "input.h"
#include "netmanager.h"
#include "connection.h"

#define DEFAULT_PORT 36738
#define TIMEOUT_VAL 20
#define MIN_PLAYERS 3
#define LOBBY_TIME 5

using namespace std;

Interface* InterfaceFactory( int mode, Interface::Verbosity v)
{
	switch ( mode)
	{
	case 0:
		return new TextInterface( v);
		break;
	case 1:
	case 2:
	default:
		return new CursesInterface( v);
		break;
	}
}

int main( int argc, char *argv[])
{
int port = DEFAULT_PORT;
int timeout = TIMEOUT_VAL;
int lobbyTimer = LOBBY_TIME;
int minPlayers = MIN_PLAYERS;
bool log = false;
int i, x;
int verbIndex = -1;
Interface::Verbosity verbosity;
int interfaceMode = -1;
	
	InitConsole();

	//process arguments
	i = 1;
	while ( i < argc)
	{
		if ( strcmp( argv[i], "-p") == 0 )
		{
			if ( i+1 < argc)
			{
				x = atoi( argv[i+1]);
				i+=2;
				if ( x > 1000 )
					port = x;
				else
					throw "Invalid Port #";
			}
			else
			{
				throw "Invalid Arguments";
			}
		}
		else if ( strcmp( argv[i], "-t") == 0 )
		{
			if ( i+1 < argc)
			{
				x = atoi( argv[i+1]);
				i+=2;
				if ( x > 0 )
					timeout = x;
				else
					throw "Invalid Timeout #";
			}
			else
			{
				throw "Invalid Arguments";
			}
		}
		else if ( strcmp( argv[i], "-l") == 0 )
		{
			if ( i+1 < argc)
			{
				x = atoi( argv[i+1]);
				i+=2;
				if ( x > 0 )
					lobbyTimer = x;
				else
					throw "Invalid Lobby Timer #";
			}
			else
			{
				throw "Invalid Arguments";		
			}
		}
		else if ( strcmp( argv[i], "-m") == 0 )
		{
			if ( i+1 < argc)
			{
				x = atoi( argv[i+1]);
				i+=2;
				if ( x > 0 )
					minPlayers = x;
				else
					throw "Invalid min player count";				
			}
			else
			{
				throw "Invalid Arguments";				
			}
		}
		else if ( strcmp( argv[i], "-i") == 0 )
		{
			if ( i+1 < argc)
			{
				sscanf( argv[i+1], "%d", &interfaceMode);
				i+=2;
			}
			else
			{
				throw "Invalid Arguments";
			}
		}
		else if ( strcmp( argv[i], "-v") == 0 )
		{
			if ( i+1 < argc)
			{
				x = atoi( argv[i+1]);
				i+=2;
				verbIndex = x;
			}
			else
			{
				throw "Invalid Arguments";
			}
		}
		else if ( strcmp( argv[i], "--log") == 0)
		{
			log = true;
			i++;
		}
		else
		{
			i++;
		}
	}
	
	if ( interfaceMode == -1)
	{
		interfaceMode = 0;
	}

	switch ( verbIndex)
	{
	case 0:
		verbosity = Interface::Silent;
		break;
	case 1:
		verbosity = Interface::Quiet;
		break;
	case 2:
		verbosity = Interface::Verbose;
		break;
	default:
		verbosity = Interface::Silent;
		break;
	}

	//run the main program, catching exceptions
	try
	{
	InterfacePtr iface( InterfaceFactory( interfaceMode, verbosity));
	Server s( iface, port, timeout*1000, lobbyTimer*1000, minPlayers, log);
		
		s.StartUp();
		while ( s.ExitCondition() == false)
		{
			s.Update();
			iface->Update();
		}
	}
	catch (const char *s)
	{
		fprintf( stderr, "Exception: %s\n", s);
	}
	
	RevertConsole();

	return 0;
}
