/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

netmessage.cpp

Implementation for NetManager module, which handles creation of management of
sockets and Connection objects
=============================*/

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <process.h>

typedef int socklen_t;
const int MSG_NOSIGNAL = 0;

#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

typedef  int SOCKET;
#endif

#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <list>
#include <map>
#include <assert.h>
#include <string>
#include <time.h>

#include "connection.h"
#include "netmanager.h"

using namespace std;

const unsigned int BufferSize = 17000;

struct SocketWrapper
{
	SOCKET desc;
	std::tr1::shared_ptr<ConnectionBuffer> buff;
	FILE *log;
	int id;
	
	SocketWrapper(): desc(0), buff(), log( 0), id( -1) {}
};


typedef list<SocketWrapper> SocketMap;

struct NetManagerImpl
{
	SocketMap sockets;
	SOCKET listenSocket;
	fd_set masterSet, workingSet;
	struct sockaddr_in sad; //server address
	struct sockaddr_in cad; //client address
	struct protoent *ptrp;  //protocol pointer
	struct timeval zeroTimeout;
	bool logging;
	string logPath;
	int nextSocketID;
	unsigned logCounter;	
};


NetManager::NetManager()
{
	my = new NetManagerImpl;
	FD_ZERO( &my->masterSet);
	FD_ZERO( &my->workingSet);	
	my->zeroTimeout.tv_sec = my->zeroTimeout.tv_usec = 0;
	my->listenSocket = 0;
	my->logging = false;
	my->logPath = "";
	my->nextSocketID = 0;
	my->logCounter = 0;
	
	#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(0x0101, &wsaData);
	#endif
	
	//Get the tcp protocol pointer
	if ( (my->ptrp = getprotobyname("tcp")) == (protoent*)0) 
	{
		fprintf(stderr, "cannot map \"tcp\" to protocol number");
		exit(1);
	}	
}

NetManager::~NetManager()
{
SocketMap::iterator iter;

	//close the listen socket (if open)
	if ( my->listenSocket != 0 )
		closesocket( my->listenSocket);


	//close all the connections and sockets
	for ( iter = my->sockets.begin(); iter != my->sockets.end(); ++iter)
	{
		iter->buff->Close();
		closesocket( iter->desc);
	}
}

void NetManager::Listen( int port, int queueLength)
{
#ifdef WIN32
char yes = 1;
#else
int yes = 1;
#endif

	//set up the server address structure
	memset((char *)&my->sad,0,sizeof(my->sad));
	my->sad.sin_family = AF_INET;
	my->sad.sin_addr.s_addr = INADDR_ANY;
	
	//check validity of port and convert to network order
	if (port > 0) 
	{
		my->sad.sin_port = htons((u_short)port);
	}
	else 
	{
		throw "Invalid Port Number in NetManager.Listen";
	}
	
	//create a socket
	my->listenSocket = socket(PF_INET, SOCK_STREAM, my->ptrp->p_proto);
	if (my->listenSocket < 0) 
	{
		throw "Socket creation failed in NetManager.Listen";
	}

	//set the options to allow reuse of address,
	//if this is not done then the server sometimes cannot bind an address for several minutes after another instance of the
	//server is closed
	if ( setsockopt( my->listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes)) == -1 )
	{
		throw "Cannot set socket options";
	}


	//Bind a local address to the socket
	if (bind(my->listenSocket, (struct sockaddr *)&my->sad, sizeof(my->sad)) < 0) 
	{
		throw "bind failed in NetManager.Listen";
	}
	
	if (listen(my->listenSocket, queueLength) < 0) 
	{
		throw "listen failed in NetManager.Listen";
	}
	
	FD_ZERO( &my->masterSet);
	FD_ZERO( &my->workingSet);
	FD_SET( my->listenSocket, &my->masterSet);
}

void NetManager::PollSockets()
{
	memcpy( &my->workingSet, &my->masterSet, sizeof( my->masterSet));
	select( FD_SETSIZE, &my->workingSet, 0, 0, &my->zeroTimeout);
}

void NetManager::Update()
{
int n;
SocketMap::iterator iter;
char buffer[BufferSize+1];

	PollSockets();
	for ( iter = my->sockets.begin(); iter != my->sockets.end(); )
	{
		//check the outgoing buffer, send data if it's present
		if ( iter->buff->out.Length() > 0 )
		{
			n = iter->buff->out.Read( buffer, BufferSize);
			send( iter->desc, buffer, n, MSG_NOSIGNAL);
			
			if ( iter->log)
			{
				buffer[n] = 0;
				fprintf( iter->log, "<<%u: %s\n", my->logCounter++, buffer);
				fflush( iter->log);
			}
		}
	
		//if the socket has incoming data, add it to the incoming buffer
		if ( FD_ISSET( iter->desc, &my->workingSet) )
		{
		unsigned int readSize = sizeof( buffer)-1;

			readSize = std::min( BufferSize, iter->buff->in.Window());

			n = recv( iter->desc, buffer, readSize, 0);
			if ( n == 0 || n == -1)
			{
				iter->buff->Close();
			}
			else 
			{	
				buffer[n] = 0;
				iter->buff->in.Write( buffer, n);
				
				if ( iter->log)
				{
					//fprintf( iter->log, ">>%u: %s\n", my->logCounter++, buffer);
					//fflush( iter->log);

					fprintf( iter->log, ">>%u: ", my->logCounter++);
					for ( int i = 0; i < n; i++)
						fprintf( iter->log, "%c", buffer[i]);
					fprintf( iter->log, "\n");
					fflush( iter->log);
				}
			}
		}
		
		//check to see if the connection has been closed, in which case remove it from our socket set and whatnot
		if ( iter->buff->IsOpen() )
		{
			++iter;
		}
		else
		{
			closesocket( iter->desc);
			FD_CLR( iter->desc, &my->masterSet);//remove this socket descriptor from the socket set
			iter = my->sockets.erase( iter);
		}
	}
}

bool NetManager::HasConnectRequest()
{
	return (my->listenSocket != 0) && FD_ISSET( my->listenSocket, &my->workingSet);
}

Connection NetManager::AcceptConnectRequest()
{
SOCKET sd;
SocketWrapper s;
socklen_t alen = sizeof( my->cad);

	if ( HasConnectRequest())
	{
		if ( (sd=accept(my->listenSocket, (struct sockaddr *)&my->cad, &alen)) < 0) 
		{
			throw "NetManager.AcceptConnectRequest failed.";
		}
	
		s = AllocateSocketWrapper( sd);
		PollSockets();//repoll the sockets so HasConnectRequest will not get stuck on true
		return Connection( s.buff);
	}
	else
	{
		throw "NetManager.AcceptConnectRequest was called with no incoming connection";
	
		return Connection();
	}
}

Connection NetManager::RequestConnection( const char *host, int port)
{
struct hostent *ptrh; //host pointer for server address
SOCKET sd;
SocketWrapper s;

	memset((char *)&my->sad,0,sizeof(my->sad));
	my->sad.sin_family = AF_INET; 

	if (port > 0)
	{
		my->sad.sin_port = htons((u_short)port);
	}
	else 
	{ 
		throw "Bad port number in NetManager.RequestConnection";
	}

	//Convert host name to equivalent IP address and copy to sad.
	ptrh = gethostbyname(host);
	if ( ((char *)ptrh) == NULL ) 
	{
		throw "Invalid host name in NetManager.RequestConnection";
	}
	memcpy(&my->sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	sd = socket(PF_INET, SOCK_STREAM, my->ptrp->p_proto);
	if (sd < 0) 
	{
		throw "Socket creation failed in NetManager.RequestConnection";
	}
	
	if (connect(sd, (struct sockaddr *)&my->sad, sizeof(my->sad)) < 0) 
	{
		throw "Connect failed in NetManager.RequestConnection";
	}
	
	//allocate a socket wrapper and return a connection 
	s = AllocateSocketWrapper( sd);
	return Connection( s.buff);
}

unsigned int NetManager::ConnectionCount()
{
	return my->sockets.size();
}

SocketWrapper NetManager::AllocateSocketWrapper( int sd)
{
SocketWrapper s;

	s.desc = sd;
	s.id = my->nextSocketID++;
	if ( my->logging)
	{
	char fname[100];
		
		sprintf( fname, "%ssocket%d.log", my->logPath.c_str(), s.id);
		s.log = fopen( fname, "w");
	}
	s.buff.reset( new ConnectionBuffer( BufferSize, BufferSize));
	my->sockets.push_back( s);
	FD_SET( (SOCKET)sd, &my->masterSet);
	
	return s;
}

void NetManager::StartLogging( const string &base)
{
#ifdef WIN32
const char slash = '\\';
int p = _getpid();
const string redirect = "2> nul";
#else
const char slash = '/';
int p = getpid();
const string redirect = "&> /dev/null";
#endif

time_t now;
struct tm tstruct;
char buf1[100];
char buf[200];

	//generate a timestamp string
	now = time( 0);
	tstruct = *localtime( &now);
	strftime( buf1, sizeof( buf1), "%m-%d-%H%M%S", &tstruct);
	sprintf( buf, "%s-%d", buf1, p);
	
	//create the directory where the logs for this session will live
	my->logging = true;
	my->logPath = base + slash + buf + slash;
	system( ("mkdir " + my->logPath + " 2> nul").c_str());//this is evil but i'm doing it anyway. mwahahaha
}
