#ifndef INSTRUMENT_
#define INSTRUMENT_

#include "Note.cpp"
#include <cstdint>
#include <cmath>
#include <iostream>

using namespace std;

class Instrument{
	public:
		std::string name;
		int type = 1;
		int channel = 0;
		bool enabled = 0;
		Note notes[128];
		std::vector<int16_t> sample;
		int startLoopIndex = 0;
		int startLoopOffset = 0;
		int endLoopIndex = 0;
		int endLoopOffset = 0;
		int loopMode;

		double delayTime;
		double attackTime;
		double holdTime;
		double decayTime;
		double releaseTime;
		double startVol;
		double sustainVol;
		double vibratoFreq;
		double vibratoVol;

		int coarseTune;
		int fineTune;

		int preSRate = 0;
		
		Instrument();
		~Instrument();
		
		void updateNoteValues();
		virtual uint32_t Output(Note*,double,unsigned int);
				
};

#endif
