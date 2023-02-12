#ifndef SYNTH_H_
#define SYNTH_H_

#include <vector>
#include "Note.cpp"
#include "Instrument.h"
#include "SoundFontManager.h"

class Synth{
	public:
		
		SoundFontManager *sfMan;
		std::vector<Instrument*> channels;

		double attackTime;
		double decayTime;
		double releaseTime;
		double startVol;
		double sustainVol;
		double vibratoFreq;
		double vibratoVol;

		unsigned int sampleRate;

		Synth(int);
		int newChannel(std::string);
		int loadChannel(std::string,int);
		uint32_t playNote(int,Note&,double);
};

#endif
