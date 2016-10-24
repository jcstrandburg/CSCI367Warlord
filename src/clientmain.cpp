/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

clientmain.cpp

Location of the main() function for the client application
Basically just reads command line arguments and sets up the Client object then passes off responsibility
=============================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

#include "wlconnection.h"
#include "netmessage.h"
#include "netmanager.h"
#include "input.h"
#include "interface.h"
#include "cursesinterface.h"
#include "clientinterfaceadapter.h"
#include "clientplaystate.h"
#include "client.h"
#include "textinterface.h"


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
string addr = "localhost";
int port = 36738;
int interfaceMode = -1;
bool autoMode = true;
bool log = false;
int i, x;
string username = "";
int verbIndex = -1;
Interface::Verbosity verbosity;

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
		else if ( strcmp( argv[i], "-s") == 0 )
		{
			if ( i+1 < argc)
			{
				addr = argv[i+1];
				i+=2;
			}
			else
			{
				throw "Invalid Arguments";
			}
		}
		else if ( strcmp( argv[i], "-u") == 0 )
		{
			if ( i+1 < argc)
			{
				username = argv[i+1];
				i+=2;
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
		else if (strcmp( argv[i], "-m") == 0)
		{
			autoMode = false;
			i++;
		}
		else if (strcmp( argv[i], "--log") == 0)
		{
			log = true;
			i++;
		}
		else
		{
			i++;
		}
	}
	
	while ( username == "")
	{
		if ( autoMode)
		{
			username = "Fishlips";
		}
		else
		{
			printf( "Enter Name: ");
			cin>>username;
		}
	}
	
	if ( interfaceMode == -1)
	{
		if ( autoMode)
			interfaceMode = 0;
		else
			interfaceMode = 2;
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
		if ( autoMode)
				verbosity = Interface::Silent;
			else
				verbosity = Interface::Verbose;
		break;
	}

	InitConsole();
	printf( "Connecting to %s as %s\n", addr.c_str(), username.c_str());
	try
	{
	InterfacePtr iface( InterfaceFactory( interfaceMode, verbosity));
	Client client( iface, autoMode, log);

		client.ConnectAs( addr.c_str(), port, username);

		while ( !client.ExitCondition() )
		{
			client.Update();
			iface->Update();
		}
	}
	catch ( const char *s)
	{
		fprintf( stderr, "Exception: %s\n", s);
	}
	RevertConsole();
}
