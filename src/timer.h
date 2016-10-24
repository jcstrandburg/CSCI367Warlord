/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

timer.h

Specification for the timer class, which provides a millisecond specific timer functionality
for timeouts and such
=============================*/

#ifndef TIMER_H
#define TIMER_H

struct TimerImp;

class Timer
{
public:
	Timer();
	~Timer();
unsigned int GetMsec();
void Start();
void Reset();
inline void Restart() { Reset(); Start(); }
void Pause();
bool IsPaused();

private:
TimerImp *my;	

};

#endif /*TIMER_H_*/
