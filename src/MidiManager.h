#ifndef MIDIMANAGER_H_
#define MIDIMANAGER_H_

#include <vector>
#include <string>
#include <fstream>

#include "synth.h"

class MidiManager{
	public:
		struct MtrkEvent {
			int deltaTime = -1;
			int channel = 0;
			int program = 0;
			int status;
			int note = 0;
			int velocity = 0;
			bool enabled = 0;
		};
		struct Mtrk{
			char trackType[4];
			int trackLength = 0;
			int pos = 0;
			int lastTick = 0;
			std::vector<MtrkEvent> trackEvents;
		};
		
		std::vector<Mtrk> tracks;
		int division;
		int tempo;
		int numChannels;
		double timer;
		int ticks;

		Synth *synth;

		MidiManager();
		void LoadFile(std::string);
		
		void reset();
		void clock(double dTime);
};

#endif
