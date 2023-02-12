#include "TriangleWave.h"
#include <iostream>

using namespace std;

TriangleWave::TriangleWave(){
	name = "Triangle";
}

uint32_t TriangleWave::Output(Note *note,double dTime,unsigned int sRate){
	double vibrato = note->vibratoAmp*sin(2*3.14159*note->vibratoFreq*dTime);
	double out = (asin(sin(2.0*3.14159*note->freq*dTime-vibrato)))*note->Output(sRate,dTime);

	uint16_t preOut = out*0x7FFF;
	uint32_t sampleOut = preOut*0x10000+preOut;
	return sampleOut;
}
