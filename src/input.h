/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

input.h

Specification for some cross platform input helper functions
=============================*/

#ifndef INPUT_H
#define INPUT_H

#include <string>

void InitConsole();
void RevertConsole();
int KeyPressed();
int GetChar();
void GetLine( std::string &s);

#endif
