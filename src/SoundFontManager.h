#pragma once

#include <cstdint>
#include <vector>
#include <string>

class SoundFontManager{
	public:
		double attackTime;
		double decayTime;
		double releaseTime;

		int iBagLength;
		int iGenLength;

		struct Instr{
			char name[20];
			uint16_t iBagIndex;
		};

		struct iBag{
			uint16_t iGenIndex;
			uint16_t iModIndex;
		};

 		struct Generator{
			int operation = -1;
			uint16_t genAmount;
		};
	
		struct Smpl{
			char name[20];
			unsigned int startIndex;
			unsigned int endIndex;
			unsigned int startLoopIndex;
			unsigned int endLoopIndex;
			unsigned int sampleRate;
			uint8_t originalPitch;
			int8_t pitchCorrection;
			uint16_t sampleLink;
			int sampleType;
		};

		struct InstrumentZone{
			std::string name;
			bool global = true;
			double delayTime = 0.01;
			double attackTime = 0.01;
			double holdTime = 0.01;
			double decayTime = 0.01;
			double releaseTime = 0.01;
			double startVol = 1.0;
			double sustainVol = 1.0;
			std::vector<double> sample;
			int startOffset = 0;
			int endOffset = 0;
			unsigned int sampleRate = 48000;
			unsigned int startLoopIndex;
			int startLoopOffset = 0;
			unsigned int endLoopIndex;
			int endLoopOffset = 0;
			int keyRangeLo = 0;
			int keyRangeHi = 127;
			int rootNote = -1;
			int8_t pitchCorrection = 0;
			int sampleMode = 0;
			int coarseTune = 0;
			int fineTune = 0;
			int sampleType = 1;
			int pan = 0;
		};

		struct Instrument{
			std::vector<InstrumentZone> zones;
			std::string name;
		};

		std::vector<Instr> instrList;
		std::vector<iBag> zoneList;
		std::vector<Generator> generators;
		std::vector<Smpl> smplList;	

		uint8_t *samples;
		std::vector<Instrument> instruments;
		
		

		SoundFontManager();
		~SoundFontManager();
		
		int loadInstrument(std::string);
		int loadInstrument(int);
		void PrintInstrumentNames();
};

