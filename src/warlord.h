/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

warlord.h

The place where we stick any globally required forward declarations or portability code, etc
=============================*/

#ifndef WARLORD_H
#define WARLORD_H

#include <string>

namespace Errors
{
	enum ErrorCode
	{
		UnknownError = 0,
		IllegalPlay = 10,
		DifferingFaces = 11,
		LowFaceValue = 12,
		LowCardCount = 13,
		CardNotInHand = 14,
		OutOfTurn = 15,
		InvalidFirstTurn = 16,
		DuplicatedCard = 17,
		PassOnStart = 18,
		Timeout = 20,
		BadMessage = 30,
		NotInGame = 31,
		ExcededLength = 32,
		UnknownMsgType = 33,
		MalformedMessage = 34,
		ChatFlood = 60,
		IllegalSwap = 70,
		CannotConnect = 80,
		ServerFull = 81,
	};
};

const unsigned int clientNameLength = 8;

	
#define null 0


#endif
