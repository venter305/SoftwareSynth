#include "SineWave.h"
#include <iostream>

using namespace std;

SineWave::SineWave(){
	name = "Sine";
}

uint32_t SineWave::Output(Note *note,double dTime,unsigned int sRate){
	double vibrato = note->vibratoAmp*sin(2*3.14159*note->vibratoFreq*dTime);
	double out = (sin(2.0*3.14159*note->freq*dTime-vibrato));
	out *= note->Output(sRate,dTime);
	
	uint16_t preOut = out*0x7FFF;
	uint32_t sampleOut = preOut*0x10000+preOut;
	return sampleOut;
}
