/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

cursesdisplays.cpp

Implementation for display objects used by the curses based interface
=============================*/
#include <curses.h>

#include "input.h"
#include "cursesdisplays.h"

DisplayBase::DisplayBase( unsigned int x, unsigned int y, unsigned int w, unsigned int h): myX( x), myY( y), myW( w), myH( h)
{
	lines.resize( h, string());
	win = newwin( h, w, y, x);
	wbkgd( win, COLOR_PAIR( 2));
	wrefresh( win);
}

DisplayBase::~DisplayBase()
{
	delwin( win);
}

void DisplayBase::RefreshLine( int l, bool emphasis)
{
int oldx, oldy;

	if ( emphasis)
	{
		wattron( win, A_BOLD);
	}
	
	getyx( stdscr, oldy, oldx);

	wmove( win, l, 0);
	wclrtoeol( win);		
	mvwaddstr( win, l, 0, lines[l].c_str());
	wrefresh( win);
	
	move( oldy, oldx);

	if ( emphasis)
	{
		wattroff( win, A_BOLD);
	}
}	

void DisplayBase::SetLine( unsigned index, const string &s, bool emphasis)
{
	lines[index] = s;
	RefreshLine( index, emphasis);
}

void DisplayBase::Clear()
{
unsigned int i;

	for ( i = 0; i < lines.size(); i++)
	{
		SetLine( i, "");
	}
}

ScrollingDisplay::ScrollingDisplay( unsigned int x, unsigned int y, unsigned int w, unsigned int h): DisplayBase( x, y, w, h)
{
}

void ScrollingDisplay::Output( const string &s)
{
unsigned int i;

	//if the string is wider than the width of the display, break it into chunks and recurse on it
	if ( s.size() > myW )
	{
		for ( i = 0; i < s.size(); i+=myW)
		{
			Output( s.substr(i, myW));
		}
		return;
	}

	//base case
	for ( i = 0; i < lines.size()-1; i++)
	{
		lines[i] = lines[i+1];
		RefreshLine( i);
	}
	lines[ lines.size()-1] = s;
	RefreshLine( lines.size()-1);
}




TickerDisplay::TickerDisplay( unsigned int x, unsigned int y, unsigned int w): DisplayBase( x, y, w, 1), delay( 60), pause( 1500)
{	
	timer.Restart();
}

void TickerDisplay::SetTimings( unsigned int d, unsigned int p)
{
	delay = d;
	pause = p;
}

void TickerDisplay::Add( const string &s)
{
	while ( lines[0].size() < myW )
		lines[0] += " ";

	lines[0] += "    ";
	lines[0] += marker;
	lines[0] += s;
	timer.Start();
	RefreshLine( 0);
}

void TickerDisplay::Update()
{
unsigned int ticks;
float bias;

	if ( lines[0].size() > 0)
	{
		ticks = timer.GetMsec();
		if ( lines[0].size() > myW)
		{
			bias = (float)myW/lines[0].size();
		}
		else
		{
			bias = 1.0;
		}

		
		if ( (lines[0][0] != marker && ticks > (unsigned int)(delay*bias)) || (ticks > (unsigned int)(pause*bias)) )
		{
			lines[0] = lines[0].substr( 1, lines[0].size()-1);
			RefreshLine( 0);
			timer.Restart();
		}
	}
	else
	{
		timer.Reset();
		timer.Pause();
	}
}



TextInDisplay::TextInDisplay( unsigned int x, unsigned int y, unsigned int w): DisplayBase( x, y, w, 1), active( false), hasInput( false)
{	
}

void TextInDisplay::Activate( const string &s)
{
	prompt = s;
	instring = "";
	active = true;
	curs_set( 2);
	Rerender();
}

void TextInDisplay::Deactivate()
{
	curs_set( 0);
	active = false;
	Rerender();
}

bool TextInDisplay::Active()
{
	return active;
}	

void TextInDisplay::Update()
{
int c;
	
	if ( !active)
		return;
	
	if ( KeyPressed())
	{
		c = getch();
		switch ( c)
		{
		case 27:
			instring = "";
			Deactivate();
			break;
		case 8://backspace
		case 263:
			instring = instring.substr( 0, instring.size()-1);
			break;
		case 10:
		case 13:
		case 459:
			SetLine( 0, "");
			hasInput = true;
			Deactivate();
			return;
		default:
			instring.push_back( (char)c);
			break;
		}
	}
	Rerender();
}

void TextInDisplay::Rerender()
{
string s;

	if ( active)
		s = prompt + ": " + instring;
	else
		s = "";
	
	if ( s.size() > myW-1 )
		s = s.substr( s.size()-(myW-1), myW-1);
	SetLine( 0, s);
	move( myY, myX+s.size());	
}

bool TextInDisplay::HasInput()
{
	return hasInput;
}

void TextInDisplay::GetInput( string &s)
{
	if ( !hasInput)
		return;
	
	hasInput = false;
	s = instring;
	instring = "";
}



ListDisplay::ListDisplay( unsigned int x, unsigned int y, unsigned int w, unsigned int h): DisplayBase( x, y, w, h)
{
}

void ListDisplay::Add( const string &s)
{
	items.insert( s);
	Rerender();
}

void ListDisplay::Remove( const string &s)
{
	items.erase( s);
	Rerender();
}

void ListDisplay::Rerender()
{
unsigned int i;
set<string>::iterator iter;

	iter = items.begin();
	for ( i = 0; i < myH; i++)
	{
		if ( iter == items.end())
		{
			SetLine( i, "");
		}
		else
		{
			SetLine( i, *iter);				
			++iter;				
		}
	}
}

bool ListDisplay::Contains( const string& s)
{
	return items.find( s) != items.end();
}
