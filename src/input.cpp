/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

input.cpp

Implementation for some cross platform input helper functions
=============================*/

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/time.h>

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

#ifndef WIN32

static struct termios oldIOS, newIOS;

void changemode(int dir)
{
  static struct termios oldt, newt;
 
  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit_linux (void)
{
  struct timeval tv;
  fd_set rdfs;
 
  tv.tv_sec = 0;
  tv.tv_usec = 0;
 
  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);
 
  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);
}

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &oldIOS); /* grab old terminal i/o settings */
  newIOS = oldIOS; /* make new settings same as old settings */
  newIOS.c_lflag &= ~ICANON; /* disable buffered i/o */
  newIOS.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newIOS); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &oldIOS);
}

/* Read 1 character - echo defines echo mode */
char getch_linux(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

#else

void changemode( int x)
{
	//do nothing
}

#endif


void InitConsole()
{
	changemode( 1);
}

void RevertConsole()
{
	changemode( 0);
}

int KeyPressed()
{
	#ifdef WIN32
	return kbhit();
	#else
	return kbhit_linux();
	#endif
}

int GetChar()
{
	#ifdef WIN32
	return getch();
	#else
	return getch_linux( 0);
	#endif
}

void GetLine( std::string &s)
{
	changemode( 0);
	getline( std::cin, s);
	changemode( 1);
}
