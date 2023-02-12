#ifndef NOTE_
#define NOTE_

#include <vector>
#include <iostream>

using namespace std;

class Note{
	struct Sample{
		std::vector<double> smpl;
		int sampleIndex = 0;

		int loopMode = 0;

		int startLoopIndex = 0;
		int startLoopOffset = 0;
		int endLoopIndex = 0;
		int endLoopOffset = 0;
		
		int sampleRate = 44100;	
		int preSRate = 44100;

		int sampleType = 1;
		int pan = 0;
	};	
	
	public:
		double freq;

		int rootNote;
			
		double delayTime;
		double attackTime;
		double holdTime;
		double decayTime;
		double releaseTime;

		double sustainVolume;
		double startVolume;

		double vibratoFreq;
		double vibratoAmp;
	
		int coarseTune;
		int fineTune;

		std::vector<Sample> samples;
		

		bool state;
		bool notePressed;
		bool noteOn;
		bool noteReleased;
		bool sustain;
		
		double lifeTime;

		double outVolume;
		double volume;
	
		bool resampled;

		Note(){
			lifeTime = 0;
			freq = 0;
			rootNote = 60;
			delayTime = 0.01;
			attackTime = 0.01;
			holdTime = 0.01;
			decayTime = 0.01;
			releaseTime = 0.01;
			sustainVolume = 1;
			startVolume = 1;
			vibratoFreq = 0;
			vibratoAmp = 0;
			coarseTune = 0;
			fineTune = 0;
			state = false;
			notePressed = false;
			noteReleased = false;
			noteOn = false;
			sustain = false;
			outVolume = 0;
			volume = 1; 
			resampled =  false;
		}

		void setNoteState(bool state){
			if (state == true){
				if (notePressed == false){
					for (int i = 0;i < samples.size();i++)
						samples[i].sampleIndex = 0;
				}
				notePressed  = true;
				noteOn = true;
				noteReleased = false;
			}
			else{
				notePressed = false;
				noteReleased = true;
				sustain = false;
			}
		}	

		double Output(unsigned int sRate,double dTime){
			if (notePressed == true){
				if (outVolume < startVolume && sustain == false)
					if (lifeTime < 1)
						lifeTime += (1/(delayTime*sRate));
					else
						outVolume += (1/(attackTime*sRate));
				else if (outVolume > sustainVolume){
					if (lifeTime < 2)
						lifeTime += (1/(holdTime*sRate));
					else
						outVolume -= (1/(decayTime*sRate));
					sustain = true;
				}
			}
			else if(noteReleased == true && noteOn == true){
				if (outVolume > 0)
					outVolume -= (1/(releaseTime*sRate));
				else{
					for (int i = 0;i < samples.size();i++)
						samples[i].sampleIndex = 0;
					noteOn = false;
					lifeTime = 0;
				}
			}
			
			if (outVolume > 1)outVolume = 1;			

			return outVolume;
		}

		uint32_t GetSample(){
			uint32_t out = 0;
			double left = 0;
			double right = 0;
			
			for (int i=0;i<samples.size();i++){
				int sIndex = samples[i].sampleIndex;
				int pan = samples[i].pan;
				double tmpOut = 0;
				if (samples[i].loopMode){
					if (sIndex < samples[i].endLoopIndex){
						tmpOut += (double)samples[i].smpl[sIndex]/0x7FFF;
						samples[i].sampleIndex++;
					}
					else
						samples[i].sampleIndex = samples[i].startLoopIndex;
				}
				else{
					if (sIndex < samples[i].smpl.size()){
						tmpOut += (samples[i].smpl[sIndex]/0x7FFF)/samples.size();
						samples[i].sampleIndex++;
					}
				}

				right += (tmpOut-tmpOut*(pan/-500.0))/2.0;
				left += (tmpOut-tmpOut*(pan/500.0))/2.0;
			}

			right /= samples.size();
			left /= samples.size();
			

			return (uint16_t)(left*0x7FFF)*0x10000+(uint16_t)(right*0x7FFF);
		}

		void NewSample(vector<double> smpls,int prevSRate,int sRate,int sPan,int sType,int lMode,int slIndex, int elIndex){
			Sample sample;
			sample.smpl = smpls;
			sample.preSRate = prevSRate;
			sample.sampleRate = sRate;
			sample.pan = sPan;
			sample.sampleType = sType;
			sample.loopMode = lMode;
			sample.startLoopIndex = slIndex;
			sample.endLoopIndex = elIndex;
			samples.push_back(sample);
		}
};

#endif
