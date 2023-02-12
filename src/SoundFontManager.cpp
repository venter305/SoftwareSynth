#include "SoundFontManager.h"
#include <fstream>
#include <iostream>
#include <cmath>

using namespace std;

SoundFontManager::SoundFontManager(){
	attackTime = 0.01;
	decayTime = 0.01;
	releaseTime = 0.01;

	ifstream sfFile;
	sfFile.open("./SoundFonts/FluidR3_GM.sf2",ios::binary);

	auto printCharArray = [](char *c,int numElements){
		for (int i=0;i<numElements;i++)
			cout << c[i];

		cout << endl;
	};

	auto compareCharArray = [&](char *c1,string s2,int numElements){	
		
		return !s2.compare(string(c1,4));
	};

	int iBagIndex = 0;
	int iBagSize = 0;
	int iGenIndex = 0;
	int iGenSize = 0;
	vector<int>smplID;

	char lenData[4];
//Header
	char riffType[4];
	unsigned int riffLength = 0;
	char riffName[4];
	sfFile.read(riffType,4);
	sfFile.read(lenData,4);
	riffLength = (unsigned char)lenData[3]*0x1000000 + (unsigned char)lenData[2]*0x10000 + (unsigned char)lenData[1]*0x100 + (unsigned char)lenData[0];
	sfFile.read(riffName,4);
//	
////Lists
	for (int i=0;i<3;i++){
		char listType[4];
		unsigned int listLen1 = 0;
		char listName[4];
		sfFile.read(listType,4);
		sfFile.read(lenData,4);
		listLen1 = (unsigned char)lenData[3]*0x1000000 + (unsigned char)lenData[2]*0x10000 + (unsigned char)lenData[1]*0x100 + (unsigned char)lenData[0];
		sfFile.read(listName,4);

		
		int index = 0;
		while (index < listLen1-4){
			char chkName[4];
			unsigned int chkLen = 0;
			sfFile.read(chkName,4);
			sfFile.read(lenData,4);
			chkLen = (unsigned char)lenData[3]*0x1000000 + (unsigned char)lenData[2]*0x10000 + (unsigned char)lenData[1]*0x100 + (unsigned char)lenData[0];

			index += chkLen+8;
			if (compareCharArray(listName,"sdta",4)){
				if (compareCharArray(chkName,"smpl",4)){
					samples = new uint8_t[chkLen];
					sfFile.read((char*)samples,chkLen);
				}
			}
			else if (compareCharArray(listName,"pdta",4)){
				if (compareCharArray(chkName,"inst",4)){
					for (int i=0;i<chkLen;i+=22){
						Instr instrument;
						sfFile.read(instrument.name,20);
				
						char indexData[2];
						sfFile.read(indexData,2);
						instrument.iBagIndex = (unsigned char)indexData[1]*0x100 + (unsigned char)indexData[0];
						
//						if (instrument.iBagIndex <= 1977)
//							cout << instrument.iBagIndex << endl;

//						if (instrument.iBagIndex <= 367)
//							cout << instrument.name << endl;
//					
						instrList.push_back(instrument);
					}
				}
				else if (compareCharArray(chkName,"ibag",4)){
					iBagLength = chkLen;
					for (int i=0;i<chkLen;i+=4){
						iBag instrBag;
						char iGenData[2];
						sfFile.read(iGenData,2);
						instrBag.iGenIndex = (unsigned char)iGenData[1]*0x100 + (unsigned char)iGenData[0];

//						if (instrBag.iGenIndex <= 22463)
//							cout << instrBag.iGenIndex << endl;

//						if (instrBag.iGenIndex <= 15621)
//							cout << zoneList.size() << endl;
						
						sfFile.read(iGenData,2);
						instrBag.iModIndex = (unsigned char)iGenData[1]*0x100 + (unsigned char)iGenData[0];
						zoneList.push_back(instrBag);
					}	
				}
				else if (compareCharArray(chkName,"igen",4)){
					iGenLength = chkLen;
					for (int i=0;i<chkLen;i+=4){
						Generator gen;
						char genData[2];
						sfFile.read(genData,2);
						gen.operation = (unsigned char)genData[1]*0x100+(unsigned char)genData[0];
						
//						if (gen.operation == 2)	
//							cout << generators.size() << endl;
						
						char valData[2];
						sfFile.read(valData,2);
						gen.genAmount = (unsigned char)valData[1]*0x100+(unsigned char)valData[0];
						generators.push_back(gen);
					}
				}
				else if (compareCharArray(chkName,"shdr",4)){
					for (int i=0;i<chkLen;i+=46){
						Smpl sample;
						sfFile.read(sample.name,20);
						char data[4];
						sfFile.read(data,4);
						sample.startIndex = ((unsigned char)data[3]*0x1000000 + (unsigned char)data[2]*0x10000 + (unsigned char)data[1]*0x100 + (unsigned char)data[0])*2;
						sfFile.read(data,4);
						sample.endIndex = ((unsigned char)data[3]*0x1000000 + (unsigned char)data[2]*0x10000 + (unsigned char)data[1]*0x100 + (unsigned char)data[0])*2;
						sfFile.read(data,4);
						sample.startLoopIndex = ((unsigned char)data[3]*0x1000000 + (unsigned char)data[2]*0x10000 + (unsigned char)data[1]*0x100 + (unsigned char)data[0])*2;
						sfFile.read(data,4);
						sample.endLoopIndex = ((unsigned char)data[3]*0x1000000 + (unsigned char)data[2]*0x10000 + (unsigned char)data[1]*0x100 + (unsigned char)data[0])*2;
						sfFile.read(data,4);
						sample.sampleRate = (unsigned char)data[3]*0x1000000 + (unsigned char)data[2]*0x10000 + (unsigned char)data[1]*0x100 + (unsigned char)data[0];
						sfFile.read(data,2);
						sample.originalPitch = data[0];
						sample.pitchCorrection = data[1];
						sfFile.read(data,2);
						sample.sampleLink = (unsigned char)data[1]*0x100+(unsigned char)data[0];
						sfFile.read(data,2);
						sample.sampleType = (unsigned char)data[1]*0x100+(unsigned char)data[0];
			
						smplList.push_back(sample);
					}
				}
				else
					sfFile.ignore(chkLen,EOF);
			}
			else
				sfFile.ignore(chkLen,EOF);
		}
	}

	sfFile.close();
}

SoundFontManager::~SoundFontManager(){
	delete samples;
}

int SoundFontManager::loadInstrument(string name){
	for (int i=0;i<instrList.size();i++){
		if (instrList[i].name == name){
			return loadInstrument(i);
			break;
		}
	}
	
	return -1;
}

int SoundFontManager::loadInstrument(int index){
	Instrument instr;
	instr.name = string(instrList[index].name);
	int bagIndex = instrList[index].iBagIndex;
	int bagSize = (index != instrList.size()-1)?(instrList[index+1].iBagIndex-bagIndex):0;
	for (int i=0;i<bagSize;i++){
		InstrumentZone instrZone;
		if (i && instr.zones[0].global)
			instrZone = instr.zones[0];
		instrZone.name = instr.name+'_'+to_string(i);
		int genIndex = zoneList[bagIndex+i].iGenIndex;
	unsigned	int genSize = (i != zoneList.size()-1)?(zoneList[(bagIndex+i)+1].iGenIndex-genIndex):(iGenLength-genIndex*4);
		for (int j=0;j<genSize;j++){
			uint16_t genAmount = generators[genIndex+j].genAmount;
			switch(generators[genIndex+j].operation){
				case 0:
					instrZone.startOffset = genAmount;
					break;
				case 1:
					instrZone.endOffset = (int16_t)genAmount;
					break;
				case 2:
					instrZone.startLoopOffset = (int16_t)genAmount;
					break;
				case 3:
					instrZone.endLoopOffset = (int16_t)genAmount;
					break;
				case 17:
					instrZone.pan = (int16_t)genAmount;
					break;
				case 33:
					instrZone.delayTime = pow(2,((int16_t)genAmount/1200.0));
					break;
				case 34:
					instrZone.attackTime = pow(2,((int16_t)genAmount/1200.0));
					break;
				case 35:
					instrZone.holdTime = pow(2,((int16_t)genAmount/1200.0));
					break;
				case 36:
					instrZone.decayTime = pow(2,((int16_t)genAmount/1200.0));
					break;
				case 37:
					instrZone.sustainVol = 1 - (genAmount/1000.0);
					break;
				case 38:
					instrZone.releaseTime = pow(2,((int16_t)genAmount/1200.0));
					break;
				case 43:
					instrZone.keyRangeHi = (genAmount&0xFF00)>>8;
					instrZone.keyRangeLo = genAmount&0x00FF;
					break;
				case 48:
					instrZone.startVol = 1 - (genAmount/1000.0);
					break;
				case 51:
					instrZone.coarseTune = (int16_t)genAmount;
					break;
				case 52:
					instrZone.fineTune = (int16_t)genAmount;
					break;
				case 53:
					if (genAmount < smplList.size()){
						instrZone.global = false;
						instrZone.sampleRate = smplList[genAmount].sampleRate;
						instrZone.startLoopIndex = smplList[genAmount].startLoopIndex - smplList[genAmount].startIndex;
						instrZone.startLoopIndex += instrZone.startLoopOffset;
						instrZone.endLoopIndex = smplList[genAmount].endLoopIndex-smplList[genAmount].startIndex;
						instrZone.endLoopIndex += instrZone.endLoopOffset;
						instrZone.name = string(smplList[genAmount].name,20);
						instrZone.sampleType = smplList[genAmount].sampleType;
						if (instrZone.rootNote == -1)
							instrZone.rootNote = smplList[genAmount].originalPitch;
						instrZone.pitchCorrection = smplList[genAmount].pitchCorrection;
						int sampleStart = smplList[genAmount].startIndex + instrZone.startOffset;
						int sampleEnd = smplList[genAmount].endIndex+instrZone.endOffset;
						for (int s=sampleStart;s<sampleEnd;s+=2){
						double tmpSmpl = (int16_t)((uint8_t)samples[s+1]*0x100+(uint8_t)samples[s]);
						instrZone.sample.push_back(tmpSmpl);
						}
					}
					break;
				case 54:
					instrZone.sampleMode = genAmount;
					break;
				case 58:
					if (genAmount != -1)
						instrZone.rootNote = genAmount;
					break;
			};
		}

		instr.zones.push_back(instrZone);
	}
	
	instruments.push_back(instr);

	return instruments.size()-1;
}
