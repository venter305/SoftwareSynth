#include "SquareWave.h"
#include <iostream>

using namespace std;

SquareWave::SquareWave(){
	name = "Square";
}

uint32_t SquareWave::Output(Note *note,double dTime,unsigned int sRate){
	double vibrato = note->vibratoAmp*sin(2*3.14159*note->vibratoFreq*dTime);
	double out = (sin(2.0*3.14159*note->freq*dTime-vibrato));
	if (out > 0.5)
		out = note->Output(sRate,dTime)/2;
	else
		out = -note->Output(sRate,dTime)/2;

	uint16_t preOut = out*0x7FFF;
	uint32_t sampleOut = preOut*0x10000+preOut;
	return sampleOut;
}
