#include "MidiManager.h"
#include <iostream>

using namespace std;

MidiManager::MidiManager(){
	division = 0;
	tempo = 120;
	numChannels = 0;
	timer = 0;
	ticks = 0;
}

void MidiManager::LoadFile(string name){
	for (int i=0;i<tracks.size();i++){
		tracks[i].trackEvents.clear();
	}
	
	tracks.clear();
	numChannels = 0;	

	char data[4]; 

	char type[4];
	int length = 0;
	uint16_t format = 0;
	uint16_t ntrks = 0;

	int runStatus = 0x80;

	int numBytes = 0;
	ifstream midiFile;
	midiFile.open(name,ios::binary);
	midiFile.read(type,4);
	midiFile.read(data,4);
	length = (unsigned char)data[0]*0x1000000+(unsigned char)data[1]*0x10000+(unsigned char)data[2]*0x100+(unsigned char)data[3];
	midiFile.read(data,2);
	format = (unsigned char)data[0]*0x100 + (unsigned char)data[1];
	midiFile.read(data,2);
	ntrks = (unsigned char)data[0]*0x100 + (unsigned char)data[1];
	midiFile.read(data,2);
	division = (unsigned char)data[0]*0x100 + (unsigned char)data[1];

	for (int i=0;i<ntrks;i++){
		Mtrk trk;
		midiFile.read(trk.trackType,4);
		midiFile.read(data,4);
		trk.trackLength = (unsigned char)data[0]*0x1000000+(unsigned char)data[1]*0x10000+(unsigned char)data[2]*0x100+(unsigned char)data[3];
		int index=0;
		while(index < trk.trackLength){
			char data;
			MtrkEvent trkEvent;

			//Delta Time
			int tmpVal = 0;
			midiFile.read(&data,1);
			index++;
			while (data&0x80){
				tmpVal += (data&0x7F);
				tmpVal<<=7;
				midiFile.read(&data,1);
				index++;
			}
			tmpVal += data;		
			trkEvent.deltaTime = tmpVal;

			//Event
			data = midiFile.peek();
			if (data&0x80){
				midiFile.read(&data,1);
				index++;
				trkEvent.status = (unsigned char)data;
				if (data < 0xF0)
					runStatus = (unsigned char)data;
			}
			else
				trkEvent.status = runStatus;
		
		
			char data1[2];
		
			switch(trkEvent.status&0xF0){
				case 0x80:
					midiFile.read(data1,2);
					index += 2;
					trkEvent.note = data1[0];
					trkEvent.velocity = data1[1];
					trkEvent.enabled = false;
					trkEvent.channel = (trkEvent.status&0x0F)+1;
					if (trkEvent.channel > numChannels)
						numChannels = trkEvent.channel;
					break;
				case 0x90:
					midiFile.read(data1,2);
					index += 2;
					trkEvent.note = data1[0];
					trkEvent.velocity = data1[1];
					trkEvent.enabled = (trkEvent.velocity)?true:false;
					trkEvent.channel = (trkEvent.status&0x0F)+1;
					if (trkEvent.channel > numChannels)
						numChannels = trkEvent.channel;
					break;
				case 0xA0:
					midiFile.read(data1,2);
					index += 2;
					//trkEvent.event.data.push_back(data1[0]);
					//trkEvent.event.data.push_back(data1[1]);
					break;
				case 0xB0:
					midiFile.read(data1,2);
					index += 2;
					//trkEvent.event.data.push_back(data1[0]);
					//trkEvent.event.data.push_back(data1[1]);
					break;
				case 0xC0:
					midiFile.read(data1,1);
					index++;
					//trkEvent.event.data.push_back(data1[0]);
					break;
				case 0xD0:
					midiFile.read(data1,1);
					index++;
					//trkEvent.event.data.push_back(data1[0]);
					break;
				case 0xE0:
					midiFile.read(data1,2);
					index += 2;
					//trkEvent.event.data.push_back(data1[0]);
					//trkEvent.event.data.push_back(data1[1]);
					break;
				case 0xF0:
					if (trkEvent.status == 0xF1){
						midiFile.read(data1,1);
						index++;
						//trkEvent.event.data.push_back(data1[0]);
					}
					else if (trkEvent.status == 0xF2){
						midiFile.read(data1,2);
						index += 2;
						//trkEvent.event.data.push_back(data1[0]);
						//trkEvent.event.data.push_back(data1[1]);
					}
					else if (trkEvent.status == 0xF3){
						midiFile.read(data1,1);
						index++;
						//trkEvent.event.data.push_back(data1[0]);
					}
					else if (trkEvent.status == 0xF0 || trkEvent.status == 0xF7){
						tmpVal = 0;
						midiFile.read(&data,1);
						index++;
						while (data&0x80){
							tmpVal += (data&0x7F);
							tmpVal<<=7;
							midiFile.read(&data,1);
							index++;
						}
						tmpVal += data;		
						char lenData[tmpVal];
						midiFile.read(lenData,tmpVal);
						index += tmpVal;
					}
					else if (trkEvent.status == 0xFF){
						midiFile.read(&data,1);
						index++;
						int type = data;
						tmpVal = 0;
						midiFile.read(&data,1);
						index++;
						while (data&0x80){
							tmpVal += (data&0x7F);
							tmpVal<<=7;
							midiFile.read(&data,1);
							index++;
						}
						tmpVal += data;
						char lenData[tmpVal];
						midiFile.read(lenData,tmpVal);
						if (type == 0x51){
							double msPerQuart = lenData[0]*0x10000 + lenData[1]*0x100 + lenData[2];
							tempo = 60/(msPerQuart/1000000);
						}
						index += tmpVal;
					}
					break;
			}
			
			trk.trackEvents.push_back(trkEvent);
		}
		
		tracks.push_back(trk);
	}
	
	midiFile.close();
}

void MidiManager::reset(){
	vector<Mtrk>::iterator it;
	for (it = tracks.begin();it != tracks.end();it++)
		it->pos = 0;
}	

void MidiManager::clock(double dTime){
	vector<Mtrk>::iterator trackIt;
	if (!tracks.empty()){
		if (dTime-timer >=  60.0/(division*tempo)){
			timer = dTime;
			for (trackIt = tracks.begin();trackIt != tracks.end();trackIt++){
				if ((*trackIt).pos < (*trackIt).trackEvents.size()){
					MtrkEvent mNote = (*trackIt).trackEvents[(*trackIt).pos];
					if (ticks-(*trackIt).lastTick >= mNote.deltaTime){
						(*trackIt).pos++;
						(*trackIt).lastTick = ticks;
						synth->channels[mNote.channel]->notes[mNote.note].volume = mNote.velocity/64.0;	
						synth->channels[mNote.channel]->notes[mNote.note].setNoteState(mNote.enabled);
					}
				}
			}
			ticks++;
		}
	}
}
