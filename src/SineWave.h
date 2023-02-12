#ifndef SINEINSTR_
#define SINEINSTR_

#include "Instrument.h"
#include "Note.cpp"
#include <iostream>

class SineWave : public Instrument{
	public:
		SineWave();
		int x = 100;

		uint32_t Output(Note*,double,unsigned int);
};

#endif
