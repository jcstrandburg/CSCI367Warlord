/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

cursesdisplays.h

Specification for display objects used by the curses based interface
=============================*/

#ifndef CURSESDISPLAYS_H
#define CURSESDISPLAYS_H

#include <curses.h>
#include <vector>
#include <string>
#include <set>
#include "timer.h"

using namespace std;

class DisplayBase
{
public:
	DisplayBase( unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	~DisplayBase();
	void RefreshLine( int l, bool emphasis = false);	
	void SetLine( unsigned index, const string &s, bool emphasis = false);
	void Clear();
	
protected:	
	vector<string> lines;
	WINDOW *win;
	unsigned int myX, myY, myW, myH;
};

class ScrollingDisplay: public DisplayBase
{
public:
	ScrollingDisplay( unsigned int x, unsigned int y, unsigned int w, unsigned int h);	
	void Output( const string &s);
};

class TickerDisplay: public DisplayBase
{
public:
	TickerDisplay( unsigned int x, unsigned int y, unsigned int w);	
	void SetTimings( unsigned int d, unsigned int p);	
	void Add( const string &s);	
	void Update();	
protected:
	Timer timer;
	unsigned int delay, pause;
	static const char marker = 255;	
};

class TextInDisplay: public DisplayBase
{
public:
	TextInDisplay( unsigned int x, unsigned int y, unsigned int w);
	void Activate( const string &s);
	bool Active();	
	void Deactivate();
	void Update();	
	bool HasInput();	
	void GetInput( string &s);

protected:
	void Rerender();

string prompt;
string instring;
bool active;
bool hasInput;
};

class ListDisplay: public DisplayBase
{
public:
	ListDisplay( unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	void Add( const string &s);	
	void Remove( const string &s);
	void Rerender();	
	bool Contains( const string& s);
private:
	set<string> items;
};

#endif /*CURSESDISPLAYS_H*/
