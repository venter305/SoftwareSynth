#ifndef SQUAREWAVE_H_
#define SQUAREWAVE_H_

#include "Instrument.h"

class SquareWave : public Instrument{
	public:
		SquareWave();

		uint32_t Output(Note*,double,unsigned int);
};

#endif
