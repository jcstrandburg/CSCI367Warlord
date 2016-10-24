/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

timer.cpp

Implementation for the timer class, which provides a millisecond specific timer functionality
for timeouts and such
=============================*/


#include <stdio.h>
#include <time.h>

#include "timer.h"	


#ifdef WIN_32
typedef clock_t MsecCount;
#else
#include <sys/time.h>
typedef unsigned long long int MsecCount;
#endif

struct TimerImp
{
unsigned int tickCount;
MsecCount msecBase;
bool paused;
};


MsecCount GetSystemMSec()
{
	#ifdef WIN32
	return clock();//may need to check this for possible conversions if clock does not return milliseconds, not worried about it now
	#else
struct timespec t;

	clock_gettime( CLOCK_REALTIME, &t);
	return t.tv_sec*1000 + t.tv_nsec/1000000;	
	#endif
}




Timer::Timer()
{
	my = new TimerImp;
	my->paused = true;
	my->tickCount = 0;
	my->msecBase = 0;
}

Timer::~Timer()
{
	delete my;
	my = 0;
}

unsigned int Timer::GetMsec()
{
	if ( my->paused)
		return my->tickCount;
	else
		return (GetSystemMSec() - my->msecBase);
}

void Timer::Start()
{
	if ( my->paused)
	{
		my->msecBase = GetSystemMSec() - my->tickCount;
		my->paused = false;
	}
}

void Timer::Reset()
{
	my->tickCount = 0;
	my->msecBase = GetSystemMSec();
}

void Timer::Pause()
{
	my->tickCount = GetMsec();
	my->msecBase = 0;
	my->paused = true;
}

bool Timer::IsPaused()
{
	return my->paused;
}
