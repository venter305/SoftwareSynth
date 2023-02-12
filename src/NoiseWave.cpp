#include "NoiseWave.h"
#include <iostream>

using namespace std;

NoiseWave::NoiseWave(){
	name = "Noise";
}

uint32_t NoiseWave::Output(Note *note,double dTime,unsigned int sRate){
	double out = sin(rand())*note->Output(sRate,dTime);
	
	uint16_t preOut = out*0x7FFF;
	uint32_t sampleOut = preOut*0x10000+preOut;
	return sampleOut;
}
