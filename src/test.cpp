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
#include "netmessage.h"
#include "input.h"

string serveraddr = "-s localhost -p 36738";

class ProcessSpawner
{
public:
	ProcessSpawner()
	{
		ttl = 0;
#ifdef WIN32
		ZeroMemory( &si, sizeof( si));
		ZeroMemory( &pi, sizeof( pi));
#else
		pid = 0;
#endif
	}
	
	~ProcessSpawner()
	{
		Kill();
	}
	
void	Spawn( const char *addr, int timeToLive = 0)
{
	ttl = timeToLive;	
	
#ifdef WIN32
char buffer[ strlen(addr)+1];
char* args;

	ZeroMemory( &si, sizeof( si));
	si.cb = sizeof( si);
	
	strcpy( buffer, addr);
	args = strchr( addr, ' ');
	if ( args)
	{
		buffer[ args-addr] = 0;
	}
	
	if ( CreateProcess( buffer, args, NULL, NULL, false, 0, NULL, NULL, &si, &pi))//we good
	{
		
	}
	else//we bad
	{
		
	}
#else
vector<string> tokens;
char **args;
unsigned int i;

	ExplodeString( " ", string( addr), tokens);
	args = new char*[tokens.size()+1];
	for ( i = 0; i < tokens.size(); i++)
	{
		args[i] = strdup( tokens[i].c_str());
	}
	args[ tokens.size()] = 0;

	pid = fork();
	switch ( pid)
	{
	case 0://child
		execv( args[0], args);
		break;
	default://parent
		return;
		break;
	}
#endif
}


void	Kill()
{
#ifdef WIN32
	if ( pi.hProcess != 0)
	{
		TerminateProcess( pi.hProcess, 0);
		CloseHandle( pi.hThread);
		CloseHandle( pi.hProcess);
		pi.hProcess = 0;
	}
#else
	if ( pid != 0)
	{
		kill( pid, SIGTERM);
		pid = 0;
	}
#endif
}

int	ttl;

private:
#ifdef WIN32
PROCESS_INFORMATION pi;
STARTUPINFO si;
#else
pid_t pid;
#endif
};

void Pause( int millis)
{
#ifdef WIN32
	Sleep( millis);
#else
	usleep( millis*1000);
#endif
}

void RunBots( int argc, const char* argv[])
{
#ifdef WIN32
string clientPath = "client.exe";
#else
string clientPath = "client";
#endif
vector<string> names;
vector<ProcessSpawner> s;
int numProcs = 1;
int minTime, maxTime;
unsigned int i;

	names.push_back( "Daedelus");
	names.push_back( "Minos");
	names.push_back( "Ajax");
	names.push_back( "Hector");
	names.push_back( "Achilles");
	names.push_back( "Zeus");
	names.push_back( "Athena");
	names.push_back( "Laocoon");
	names.push_back( "Hercules");
	names.push_back( "Odyseus");
	names.push_back( "Apollo");
	names.push_back( "Dionysus");
	names.push_back( "Hermes");

	
	if ( argc != 4 )
	{
		printf( "Incorrect arguments! Usage is test numClients minTime maxTime\n");
		RevertConsole();
		exit( -1);
	}
	else
	{
		sscanf( argv[1], "%d", &numProcs);
		sscanf( argv[2], "%d", &minTime);
		sscanf( argv[3], "%d", &maxTime);
	}
	
	
	srand( time(0));
	s.resize( numProcs, ProcessSpawner());
	
	while ( !KeyPressed())
	{
		for ( i = 0; i < s.size(); i++)
		{
			if ( s[i].ttl > 0)
			{
				s[i].ttl--;
			}
			else
			{
			string e = clientPath + " -u " + names[i%names.size()] + " --auto " + serveraddr;
			
				s[i].Kill();
				s[i].Spawn( e.c_str(), minTime + rand()%(maxTime-minTime) );
			}
		}
		Pause( 1000);				
	}
}

void TestExplodeString( int argc, const char *argv[])
{/*
vector<string> tokens;

	ExplodeString( "|", "slobb|00|", tokens);
	for ( unsigned i = 0; i < tokens.size(); i++)
	{
		printf( "Token %d (%s)\n", i, tokens[i].c_str());
	}*/

string delim = "|";
string s = "slobb|00||bbob";
size_t pos = 0;
size_t start =  0;
vector<string> out;

	string s2;

	out.clear();
	while ( (pos = s.find_first_of( delim, start)) != string::npos )
	{
		out.push_back( RemovePadding( s.substr( start, pos-start)));
		start = pos+1;
	}
	if ( start < s.size())
	{
		out.push_back( RemovePadding( s.substr( start, s.size()-start)));
	}

	for ( unsigned i = 0; i < out.size(); i++)
	{
		printf( "Token %d (%s)\n", i, out[i].c_str());
	}
}

int main( int argc, const char *argv[])
{
	InitConsole();

	RunBots( argc, argv);
	//TestExplodeString( argc, argv);


	RevertConsole();

	return 0;
}
