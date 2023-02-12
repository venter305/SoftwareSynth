#include <iostream>
#include "./SoundManager/soundManager.h"
#include <cmath>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include "./GraphicsEngine/graphicsEngine.h"
#include "./GraphicsEngine/GUI/panel.h"
#include "./GraphicsEngine/GUI/textObject.h"
#include "./GraphicsEngine/GUI/button.h"
#include "./GraphicsEngine/GUI/textInput.h"
#include <vector>
#include <fstream>
#include <queue>
#include "MidiManager.h"
#include "InputManager/InputManager.h"
#include "SoundFontManager.h"
#include "synth.h"
#include <map>


using namespace std;

#define SAMPLERATE 44100
#define SCREENWIDTH 550
#define SCREENHEIGHT 512

bool playMidi = false;
int numChannels = 1;

MidiManager midiMan;

SoundFontManager sfMan;

Synth synth(SAMPLERATE);

GraphicsEngine graphicsEngine(SCREENWIDTH,SCREENHEIGHT,"Synth");

SoundManager sndMan(SAMPLERATE);

InputManager inputMan;
uint32_t MakeSound(double dTime){
	if (playMidi)
		midiMan.clock(dTime);
	
	double left = 0;
	double right = 0;
	std::cout << synth.channels[0]->notes[60].noteOn << std::endl;
	for (int j=0;j<numChannels;j++){
		synth.channels[j]->enabled = true;
		if (synth.channels[j]->enabled){
			for (int i=0;i<=125;i++){
				if (synth.channels[j]->notes[i].noteOn){
					uint32_t tmpOut = synth.playNote(j,synth.channels[j]->notes[i],dTime);
					std::cout << tmpOut << std::endl;
					int16_t tmpLeft = (tmpOut&0xFFFF0000)/0x10000;
 					int16_t tmpRight = (tmpOut&0xFFFF);
					left += ((double)tmpLeft)/0x7FFF;
					right += ((double)tmpRight)/0x7FFF;
				}
			}
		}
	}
	

	std::cout << left << ' ' << right << std::endl;	
	right /= 128;
	right *= 20;

	left /= 128;
	left *= 20;

	uint32_t out = (uint16_t)(left*0x7FFF)*0x10000+(uint16_t)(right*0x7FFF);
	
	return out;
}

double StringToDouble(string *s){
	double tmpNum = 0;
	double numDecimal = 10;
	bool hasDecimal = false;
	string::const_iterator c;
	for (c = s->begin();c != s->end();c++){
		if (*c != '.'){
			if (!hasDecimal){
				tmpNum *= 10;
				tmpNum += (*c - 48);
			}
			else{
				tmpNum += ((*c-48)/numDecimal);
				numDecimal *= 10;
			}
		}
		else
			hasDecimal = true;
	}
	
	return tmpNum;
}

Panel *keys[17];
bool keySelected[17];
bool keyOn[17];

void textInputValueChange(TextInput *t){
	switch(t->id){
		case 100:
			synth.channels[0]->delayTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 101:
			synth.channels[0]->attackTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 102:
			synth.channels[0]->holdTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 103:
			synth.channels[0]->decayTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 104:
			synth.channels[0]->releaseTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 105:
			synth.channels[0]->startVol = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 106:
			synth.channels[0]->sustainVol = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 107:
			synth.channels[0]->vibratoFreq = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 108:
			synth.channels[0]->vibratoVol = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
			break;
		case 109:
			sndMan.start = false;
			if (synth.loadChannel(t->text->text,0) == -1)
				break;
			((TextInput*)graphicsEngine.gui.elements[100])->setText(to_string(synth.channels[0]->delayTime));
			((TextInput*)graphicsEngine.gui.elements[101])->setText(to_string(synth.channels[0]->attackTime));
			((TextInput*)graphicsEngine.gui.elements[102])->setText(to_string(synth.channels[0]->holdTime));
			((TextInput*)graphicsEngine.gui.elements[103])->setText(to_string(synth.channels[0]->decayTime));
			((TextInput*)graphicsEngine.gui.elements[104])->setText(to_string(synth.channels[0]->releaseTime));
			((TextInput*)graphicsEngine.gui.elements[105])->setText(to_string(synth.channels[0]->startVol));
			((TextInput*)graphicsEngine.gui.elements[106])->setText(to_string(synth.channels[0]->sustainVol));
			((TextInput*)graphicsEngine.gui.elements[107])->setText(to_string(synth.channels[0]->vibratoFreq));
			((TextInput*)graphicsEngine.gui.elements[108])->setText(to_string(synth.channels[0]->vibratoVol));
			sndMan.start = true;
			break;
		case 110:
			playMidi = false;
			midiMan.reset();
			sndMan.start = false;
			synth.loadChannel(t->text->text,1);
			sndMan.start = true;
			break;
		case 111:
			playMidi = false;
			sndMan.start = false;
			midiMan.LoadFile(t->text->text);
			sndMan.start = true;
			break;
	};
	
	
}

void loop(GLFWwindow *window,double dTime){

	inputMan.PollInputs();

	int state = inputMan.mState.btnLeft;	
	double xPos, yPos;
	glfwGetCursorPos(window,&xPos,&yPos);
	int windowX,windowY;
	glfwGetWindowSize(window,&windowX,&windowY);
	yPos = windowY-yPos;
	
	if (!playMidi){
		char inputKeys[] = "zsxdcvgbhnjm,l.;/";
		for (int i=0;i<17;i++){
			if (inputMan.GetKeyState(inputKeys[i])){
				synth.channels[0]->notes[i+60].setNoteState(true);
				keyOn[i] = true;
			}
			else{
				keyOn[i] = false;
				switch(i){
					case 1:
					case 3:
					case 6:
					case 8:
					case 10:
					case 13:
					case 15:
						if (keys[i]->checkBoundingBox(xPos,yPos)){
							keySelected[i] = true;
							if(inputMan.GetMouseButtonState(0)){
								synth.channels[0]->notes[i+60].setNoteState(true);
							}
							else
								synth.channels[0]->notes[i+60].setNoteState(false);
						}
						else {
							if (!keyOn[i]){
								keySelected[i] = false;
								synth.channels[0]->notes[i+60].setNoteState(false);
							}
						}
						break;
					default:
						if (keys[i]->checkBoundingBox(xPos,yPos)){
							int index = i-1;
							if (index < 0)
								index = 16;
							if (!(keys[(i+1)%16]->checkBoundingBox(xPos,yPos)) && !(keys[index]->checkBoundingBox(xPos,yPos))){
								keySelected[i] = true;
								if(inputMan.GetMouseButtonState(0)){
									synth.channels[0]->notes[i+60].setNoteState(true);
								}
								else
									synth.channels[0]->notes[i+60].setNoteState(false);
							}
							else {
								if (!keyOn[i]){
									keySelected[i] = false;
									synth.channels[0]->notes[i+60].setNoteState(false);
								}
							}
						}
						else {
							if (!keyOn[i]){
								keySelected[i] = false;
								synth.channels[0]->notes[i+60].setNoteState(false);
							}
						}
					break;
				}		
			}
		}
	}
	
	
	map<int,Panel*>::iterator it;
	for (it = graphicsEngine.gui.elements.begin();it != graphicsEngine.gui.elements.end();it++){
		if (it->first >= 100){
			if (it->second->checkBoundingBox(xPos,yPos) && inputMan.GetMouseButtonDown(0)){
				it->second->clickAction(xPos,yPos);
			}
			if (it->first < 200){
				if (((TextInput*)it->second)->enabled){
					if (inputMan.GetMouseButtonState(0)){
						if (!it->second->checkBoundingBox(xPos,yPos)){
							textInputValueChange(((TextInput*)it->second));
							it->second->clickAction(xPos,yPos);
						}
					}
					if (((TextInput*)it->second)->inputTimer <= 0){
						((TextInput*)it->second)->inputTimer = ((TextInput*)it->second)->inTimeMax;
						if (inputMan.GetKeyState(KEY_BACKSPACE))
							((TextInput*)it->second)->removeCharacter();
						else if(inputMan.GetKeyState(KEY_ENTER)){
							((TextInput*)it->second)->setEnabled(false);
							textInputValueChange(((TextInput*)it->second));
						}
						else if (inputMan.GetKeyState(KEY_SPACE)){
							((TextInput*)it->second)->addCharacter(' ');
						}
						else{
							for (char c : ((TextInput*)it->second)->inputMask){
								if (inputMan.GetKeyState(c) == 1){
									if(inputMan.GetKeyState(KEY_LEFTSHIFT) || inputMan.GetKeyState(KEY_RIGHTSHIFT))
										c = toupper(c);
									((TextInput*)it->second)->addCharacter(c);
								}
							}
						}
					}
					else{
						((TextInput*)it->second)->inputTimer -= dTime-((TextInput*)it->second)->prevInTime;
						((TextInput*)it->second)->prevInTime = dTime;
					}
				}
			}
		}
	}	

	graphicsEngine.gui.drawElements();
	
	for (int i=0;i<17;i++){
		switch(i){
			case 0:
			case 2:
			case 4:
			case 5:
			case 7:
			case 9:
			case 11:
			case 12:
			case 14:
			case 16:
				if (synth.channels[0]->notes[i+60].noteOn || keySelected[i]){
					keys[i]->setColor(0.8,0.8,0.8);
				}
				else
					keys[i]->setColor(1,1,1);
				keys[i]->draw();
				break;
			}		
	}

	for (int i=0;i<17;i++){
		switch(i){
			case 1:
			case 3:
			case 6:
			case 8:
			case 10:
			case 13:
			case 15:
				if (synth.channels[0]->notes[i+60].noteOn || keySelected[i])
					keys[i]->setColor(0.6,0.6,0.6);
				else
					keys[i]->setColor(0,0,0);
				keys[i]->draw();
				break;
			}		
	}
}


void MidiControl(Button *b){
	switch(b->id){
		case 200:
			playMidi = !playMidi;
			if(playMidi){
				b->text->setText("Pause");
				b->drawText();
				numChannels = 1+midiMan.numChannels;
			}
			else{
				b->text->setText("Play");
				b->drawText();
				numChannels = 1;
			}
			break;
		case 201:
			playMidi = false;
			midiMan.reset();
			break;
	};
} 
	
int main(){
	sndMan.UserFunc = MakeSound;
	
	midiMan.synth = &synth;
	
	synth.sfMan = &sfMan;
	synth.newChannel("Yamaha Grand Piano");
	//synth.newChannel("Sine");
		glfwSetInputMode(graphicsEngine.window,GLFW_STICKY_MOUSE_BUTTONS,GLFW_FALSE);

	
	Panel screen(0,0,SCREENWIDTH,SCREENHEIGHT);
	screen.setColor(0.9,0.9,0.9);
	graphicsEngine.gui.addElement(&screen);

	string fontPath = "./GraphicsEngine/freefont/FreeSerif.otf";	

	float enabledColor[3] = {1,1,1};
	float disabledColor[3] = {0.9,0.9,0.9};


	Text channel0(0,450,15,"Custom Channel:",fontPath);
	graphicsEngine.gui.addElement(&channel0);
	Text chl0Name(20,435,15,"Name: ",fontPath);
	graphicsEngine.gui.addElement(&chl0Name);
	TextInput chl0NameInput(75,435,200,15,synth.channels[0]->name);
	chl0NameInput.setStateColors(enabledColor,disabledColor);
	int nameId = graphicsEngine.gui.addElement(&chl0NameInput,109);

	string timeLabels = "Delay Time: " +
									string("\nAttack Time: ") +
									string("\nHold Time: ") +
									string("\nDecay Time: ") +
	 								string("\nRelease Time: ") + 
									string("\n\nStart Volume: ") +
									string("\nSustain Volume: ") +
									string("\n\nVibrato Freq: ") +
	 								string("\nVibrato Volume: ");

	Text ch0NoteValues(20,405,15,timeLabels,fontPath);
	graphicsEngine.gui.addElement(&ch0NoteValues);
	

	int baseY = 405;
	int yOff = 15;

	TextInput ch0delayTime(150,baseY-(yOff*0),100,15,to_string(synth.channels[0]->delayTime));
	ch0delayTime.setStateColors(enabledColor,disabledColor);
	ch0delayTime.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0delayTime,100);

	TextInput ch0attackTime(150,baseY-(yOff*1),100,15,to_string(synth.channels[0]->attackTime));
	ch0attackTime.setStateColors(enabledColor,disabledColor);
	ch0attackTime.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0attackTime, 101);

	TextInput ch0holdTime(150,baseY-(yOff*2),100,15,to_string(synth.channels[0]->holdTime));
	ch0holdTime.setStateColors(enabledColor,disabledColor);
	ch0holdTime.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0holdTime,102);
	
	TextInput ch0decayTime(150,baseY-(yOff*3),100,15,to_string(synth.channels[0]->decayTime));
	ch0decayTime.setStateColors(enabledColor,disabledColor);
	ch0decayTime.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0decayTime, 103);

	TextInput ch0releaseTime(150,baseY-(yOff*4),100,15,to_string(synth.channels[0]->releaseTime));
	ch0releaseTime.setStateColors(enabledColor,disabledColor);\
	ch0releaseTime.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0releaseTime, 104);

	TextInput ch0startVol(150,baseY-(yOff*6),100,15,to_string(synth.channels[0]->startVol));
	ch0startVol.setStateColors(enabledColor,disabledColor);
	ch0startVol.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0startVol, 105);

	TextInput ch0sustainVol(150,baseY-(yOff*7),100,15,to_string(synth.channels[0]->sustainVol));
	ch0sustainVol.setStateColors(enabledColor,disabledColor);
	ch0sustainVol.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0sustainVol, 106);

	TextInput ch0vibratoFreq(150,baseY-(yOff*9),100,15,to_string(synth.channels[0]->vibratoFreq));
	ch0vibratoFreq.setStateColors(enabledColor,disabledColor);
	ch0vibratoFreq.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0vibratoFreq, 107);

	TextInput ch0vibratoVol(150,baseY-(yOff*10),100,15,to_string(synth.channels[0]->vibratoVol));
	ch0vibratoVol.setStateColors(enabledColor,disabledColor);
	ch0vibratoVol.inputMask = "0123456789.";
	graphicsEngine.gui.addElement(&ch0vibratoVol, 108);

	Text channel1(270,450,15,"Channel 1:",fontPath);
	graphicsEngine.gui.addElement(&channel1);
	Text ch1Name(290,435,15,"Name: ",fontPath);
	graphicsEngine.gui.addElement(&ch1Name);
	
	TextInput ch1NameInput(345,435,200,15,"Yamaha Grand Piano");
	ch1NameInput.setStateColors(enabledColor,disabledColor);
	graphicsEngine.gui.addElement(&ch1NameInput,110);

	string midiPath = "bems1.mid";
	Text midiName(0,200,15,"Name: ",fontPath);
	graphicsEngine.gui.addElement(&midiName);

	TextInput midiNameInput(55,200,150,15,midiPath);
	midiNameInput.setStateColors(enabledColor,disabledColor);
	graphicsEngine.gui.addElement(&midiNameInput,111);

	Button midiPlayBtn(280,200,40,15,MidiControl);
	midiPlayBtn.text->setText("Play");
	midiPlayBtn.setBackgroundColor(0.8,0.8,0.8);
	midiPlayBtn.drawText();
	graphicsEngine.gui.addElement(&midiPlayBtn,200);

	Button midiResetBtn(330,200,45,15,MidiControl);
	midiResetBtn.text->setText("Reset");
	midiResetBtn.setBackgroundColor(0.8,0.8,0.8);
	midiResetBtn.drawText();
	graphicsEngine.gui.addElement(&midiResetBtn,201);

	midiMan.LoadFile(midiPath);
	numChannels += midiMan.numChannels;

//	synth.newChannel("Strings");
//	synth.newChannel("Strings");
//	synth.newChannel("Synth Brass 2");
//	synth.newChannel("French Horns");
//	synth.newChannel("Trombone");
//	synth.newChannel("Strings");
//	synth.newChannel("Clarinet");
//	synth.newChannel("Timpani");
//	synth.newChannel("Orchestra Hit");
//	synth.newChannel("Orchestra");
	
	for (int i=0;i<midiMan.numChannels;i++){
		synth.newChannel(ch1NameInput.text->text);
	}

	int whiteCount = 0;
	int blackCount = 0;
	int xOffset = 0;

	for (int i=0;i<17;i++){
		switch(i){
			case 0:
			case 2:
			case 4:
			case 5:
			case 7:
			case 9:
			case 11:
			case 12:
			case 14:
			case 16:
				keys[i] = new Panel(46*whiteCount,0,45,100);
				keys[i]->setColor(1,1,1);
				whiteCount++;
				break;
			default:
				if (i == 6 || i == 13)
					xOffset += 45;
				keys[i] = new Panel(46*blackCount+35+xOffset,50,20,50);
				keys[i]->setColor(0,0,0);
				blackCount++;
				break;
			}		
	}

	
	sndMan.start = true;

	glfwSwapInterval(0);
	graphicsEngine.mainLoop = loop;
	graphicsEngine.run();

	glfwTerminate();

	return 0;
} 
