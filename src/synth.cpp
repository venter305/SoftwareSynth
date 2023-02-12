#include "synth.h"
#include <iostream>
#include "resample.h"
#include "r8brain-free-src/CDSPResampler.h"
#include "SineWave.h"
#include "SquareWave.h"
#include "TriangleWave.h"
#include "NoiseWave.h"

using namespace std;

Synth::Synth(int sRate) : sampleRate(sRate){
	attackTime = 0.01;
	decayTime = 0.01;
	releaseTime = 1;
	startVol = 1;
	sustainVol = 1;
	vibratoFreq = 1;
	

}

int Synth::newChannel(string name){
	channels.resize(channels.size()+1);
	return loadChannel(name,channels.size()-1);
}


int Synth::loadChannel(string name, int channelNum){
	if (channelNum >= channels.size()){
		return -1;
	}

	if (channels[channelNum])
		delete channels[channelNum];
	
	if (name == "Sine"){
		SineWave *sine = new SineWave();
		channels[channelNum] = sine;
		return channelNum;
	}
	if (name == "Square"){
		SquareWave *square = new SquareWave();
		channels[channelNum] = square;
		return channelNum;
	}
	if (name == "Triangle"){
		TriangleWave *triangle = new TriangleWave();
		channels[channelNum] = triangle;
		return channelNum;
	}
	if (name == "Noise"){
		NoiseWave *noise = new NoiseWave();
		channels[channelNum] = noise;
		return channelNum;
	}
	
	int	instIndex = sfMan->loadInstrument(name);
	if (instIndex == -1)
		return -1;		

	Instrument *instr = new Instrument();
	instr->type = 4;
	instr->name = sfMan->instruments[instIndex].name;
	channels[channelNum] = instr;

	SoundFontManager::InstrumentZone *instrZone = &sfMan->instruments[instIndex].zones[0];
	if (sfMan->instruments[instIndex].zones[0].global){
		channels[channelNum]->delayTime = instrZone->delayTime;
		channels[channelNum]->attackTime = instrZone->attackTime;
		channels[channelNum]->holdTime = instrZone->holdTime;
		channels[channelNum]->decayTime = instrZone->decayTime;
		channels[channelNum]->releaseTime = instrZone->releaseTime;
		channels[channelNum]->startVol = instrZone->startVol;
		channels[channelNum]->sustainVol = instrZone->sustainVol;
		
		channels[channelNum]->coarseTune = instrZone->coarseTune;
		channels[channelNum]->fineTune = instrZone->fineTune;
	}
	
	
	//cout << sfMan->instruments[instIndex].name << endl;

	for (int i=0;i<sfMan->instruments[instIndex].zones.size();i++ ){
		instrZone = &sfMan->instruments[instIndex].zones[i];
		int instrSRate = instrZone->sampleRate;
		int newSRate = sampleRate;
		int noteLo = instrZone->keyRangeLo;
		int noteHi = instrZone->keyRangeHi;
		int rootNote = instrZone->rootNote;

//		cout << sfMan.instruments[0].zones[i].name << endl;
//		cout << rootNote << endl;
//		cout << noteLo << ' ' << noteHi << endl;
//		cout << (int)sfMan.instruments[0].zones[i].pitchCorrection << endl;
//		cout << sampleRate << endl;
//		cout << (int)sfMan.instruments[0].zones[i].sampleMode << endl;

		
		int inputSize = instrZone->sample.size();
		
		for (int n=noteLo;n<=noteHi;n++){
			//channels[channelNum]->notes[n].startLoopIndex = instrZone->startLoopIndex/2;
			//channels[channelNum]->notes[n].startLoopOffset = instrZone->sample.size() - channels[channelNum]->notes[n].startLoopIndex;
			//channels[channelNum]->notes[n].endLoopIndex = instrZone->endLoopIndex/2;
			//channels[channelNum]->notes[n].endLoopOffset = instrZone->sample.size() - channels[channelNum]->notes[n].endLoopIndex;

			channels[channelNum]->notes[n].delayTime = instrZone->delayTime;
			channels[channelNum]->notes[n].attackTime = instrZone->attackTime;
			channels[channelNum]->notes[n].holdTime = instrZone->holdTime;
			channels[channelNum]->notes[n].decayTime = instrZone->decayTime;
			channels[channelNum]->notes[n].releaseTime = instrZone->releaseTime;
			channels[channelNum]->notes[n].startVolume = instrZone->startVol; 	 	
			channels[channelNum]->notes[n].sustainVolume = instrZone->sustainVol;
	
			channels[channelNum]->notes[n].coarseTune = instrZone->coarseTune;
			channels[channelNum]->notes[n].fineTune = instrZone->fineTune;
			
//			if (n == rootNote)s
//				newSRate = sampleRate;
//			else
			int pitchCorrection = instrZone->pitchCorrection;
			int coarseTune = 100*instrZone->coarseTune;
			int fineTune = 100 + instrZone->fineTune;
		
			int cents = (fineTune+pitchCorrection)*(rootNote-n) - coarseTune;
			newSRate = pow(2,(cents)/1200.0)*sampleRate;
			//newSRate = sampleRate+2854*(rootNote-n);
			
			if (instrZone->sample.size())
				channels[channelNum]->notes[n].NewSample(instrZone->sample,instrSRate,newSRate,instrZone->pan,instrZone->sampleType,instrZone->sampleMode,instrZone->startLoopIndex/2,instrZone->endLoopIndex/2);
	
	
			channels[channelNum]->notes[n].resampled = false;
	
		}
	}

	return instIndex;
}

uint32_t Synth::playNote(int channel, Note &note, double dTime){

	if (!note.resampled && channels[channel]->type == 4){
		double *output;
		for (int i=0;i<note.samples.size();i++){
			if (note.samples[i].sampleRate == -1)
				note.samples[i].sampleRate = note.samples[i].preSRate;

			int preSampleSize = note.samples[i].smpl.size();
		
			r8b::CDSPResampler16 resampler(note.samples[i].preSRate,note.samples[i].sampleRate,preSampleSize);
			int outNum = resampler.process(note.samples[i].smpl.data(),preSampleSize, output);	
			note.samples[i].smpl.clear();
			for (int j=0;j<outNum;j++){
				note.samples[i].smpl.push_back(output[j]);
			
			}

	//	
	//		cout << outNum << endl;
			//note.startLoopIndex = note.startLoopIndex/((double)preSampleSize/outNum);
			//note.endLoopIndex = note.endLoopIndex/((double)preSampleSize/outNum);
			double sRatio = (double)note.samples[i].preSRate/note.samples[i].sampleRate;
			int offset = (preSampleSize/sRatio)-outNum; 
			int size = (note.samples[i].endLoopIndex-note.samples[i].startLoopIndex);
			note.samples[i].startLoopIndex = (note.samples[i].startLoopIndex/sRatio)-offset;
			note.samples[i].endLoopIndex = (note.samples[i].endLoopIndex/sRatio)-offset;
	//		cout << note.startLoopIndex << ' ' << note.endLoopIndex << endl;
	//		cout << note.sample[note.startLoopIndex] << ' ' << note.sample[note.endLoopIndex] << endl;
			note.resampled = true;
		}
	}
	
	return channels[channel]->Output(&note,dTime,sampleRate);
}
