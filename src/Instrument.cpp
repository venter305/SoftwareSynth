#include "Instrument.h"
#include <iostream>

using namespace std;

Instrument::Instrument(){
	name = "Null";
	delayTime = 0.01;
	attackTime = 0.01;
	holdTime = 0.01;
	decayTime = 0.01;
	releaseTime = 1;
	startVol = 1;
	sustainVol = 1;
	vibratoFreq = 1;
	vibratoVol = 0;
	loopMode = 0;
	coarseTune = 0;
	fineTune = 0;

	float noteList[17] = {261.63,277.18,293.66,311.13,329.63,349.23,369.99 	,392.00,415.30,440.00,466.16,493.88,523.25,554.37,587.33,622.25,659.25};

	for (int i=0;i<128;i++){
		notes[i].delayTime = delayTime;
		notes[i].attackTime = attackTime;
		notes[i].holdTime = holdTime;
		notes[i].decayTime = decayTime;
		notes[i].releaseTime = releaseTime;
		notes[i].startVolume = startVol;
		notes[i].sustainVolume = sustainVol;
		notes[i].vibratoFreq = vibratoFreq;
		notes[i].vibratoAmp = vibratoVol;
		if (i >= 60 && i <= 71)
			notes[i].freq = noteList[i-60];
		else if (i >= 48 && i <= 59)
			notes[i].freq = noteList[i-48]/2;
		else if ( i >= 72 && i <= 83)
			notes[i].freq = noteList[i-72]*2;
		else if (i >= 36 && i <= 47)
			notes[i].freq = noteList[i-36]/4;
		else if (i >= 84 && i <= 95)
			notes[i].freq = noteList[i-84]*4;
	}
}

Instrument::~Instrument(){
}

uint32_t Instrument::Output(Note *note,double dTime,unsigned int sRate){
	uint32_t out = note->GetSample();
	double noteVolume = note->Output(sRate,dTime);
	int16_t tmpRight = (out&0xFFFF);
	int16_t tmpLeft = ((out&0xFFFF0000)/0x10000);
	double right = ((double)tmpRight);
	double left = ((double)tmpLeft);
	right *= noteVolume*2;
	left *= noteVolume*2;
	
	return (uint16_t)(left)*0x10000+(uint16_t)(right);
}

void Instrument::updateNoteValues(){
	for (int i=0;i<128;i++){
		notes[i].delayTime = delayTime;
		notes[i].attackTime = attackTime;
		notes[i].holdTime = holdTime;
		notes[i].decayTime = decayTime;
		notes[i].releaseTime = releaseTime;
		notes[i].startVolume = startVol;
		notes[i].sustainVolume = sustainVol;
		notes[i].vibratoFreq = vibratoFreq;
		notes[i].vibratoAmp = vibratoVol;
	}
}
