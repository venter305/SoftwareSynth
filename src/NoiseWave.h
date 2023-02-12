#ifndef NOISEWAVE_H_
#define NOISEWAVE_H_

#include "Instrument.h"

class NoiseWave : public Instrument{
	public:
		NoiseWave();

		uint32_t Output(Note*,double,unsigned int);
};

#endif
