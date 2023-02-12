#ifndef TRIANGLEWAVE_H_
#define TRIANGLEWAVE_H_

#include "Instrument.h"

class TriangleWave : public Instrument{
	public:
		TriangleWave();

		uint32_t Output(Note*,double,unsigned int);
};

#endif
