#include <iostream>
#include "./SoundManager/soundManager.h"
#include <cmath>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include "./GraphicsEngine/graphicsEngine.h"
#include <vector>
#include <fstream>
#include <queue>
#include "MidiManager.h"
#include "SoundFontManager.h"
#include "synth.h"
#include <map>


#define SAMPLERATE 48000
#define SCREENWIDTH 550
#define SCREENHEIGHT 512



bool playMidi = false;
int numChannels = 1;

MidiManager midiMan;

SoundFontManager sfMan;

Synth synth(SAMPLERATE);

SoundManager sndMan(SAMPLERATE,"Synth");

uint32_t MakeSound(double dTime){
	if (playMidi)
		midiMan.clock(dTime);
	
	double left = 0;
	double right = 0;
	int numNotesPlaying = 1;
	for (int j=0;j<numChannels;j++){
		for (int i=0;i<=125;i++){
			if (synth.channels[j]->notes[i].noteOn){
				numNotesPlaying++;
				uint32_t tmpOut = synth.playNote(j,synth.channels[j]->notes[i],dTime);
				int16_t tmpLeft = (tmpOut&0xFFFF0000)/0x10000;
				int16_t tmpRight = (tmpOut&0xFFFF);
				left += ((double)tmpLeft)/0x7FFF;
				right += ((double)tmpRight)/0x7FFF;
			}
		}
	}
	

	right /= 125*numChannels;
	left /= 125*numChannels;
		 
	right *= 20;
	left *= 20;

	uint32_t out = (uint16_t)(left*0x7FFF)*0x10000+(uint16_t)(right*0x7FFF);
		
	return out;
}


void MidiControl(Button *b){
	switch(b->GetId()){
		case 200:
			playMidi = !playMidi;
			if(playMidi){
				b->text->SetText("Pause");
				numChannels = 1+midiMan.numChannels;
			}
			else{
				b->text->SetText("Play");
				numChannels = 1;
			}
			break;
		case 201:
			playMidi = false;
			midiMan.reset();
			break;
	};
} 
	


class MainWindow : public Window{
public:
	MainWindow(int w, int h, std::string name) : Window(w,h,name){}
	

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
	
	void OnStartup() {

		for (int i=0;i<17;i++){
			keySelected[i] = false;
			keyOn[i] = false;
		}

		sndMan.UserFunc = MakeSound;
			
		midiMan.synth = &synth;
		
		synth.sfMan = &sfMan;
		synth.newChannel("Yamaha Grand Piano");
		//synth.newChannel("Sine");

		
		std::shared_ptr<Panel> screen = std::make_shared<Panel>(0,0,SCREENWIDTH,SCREENHEIGHT);
		screen->SetColor(0.9,0.9,0.9,1.0);
		guiMan.AddElement(screen);

		std::string fontPath = "./GraphicsEngine/freefont/FreeSerif.otf";	

		float enabledColor[3] = {1,1,1};
		float disabledColor[3] = {0.9,0.9,0.9};


		std::shared_ptr<Text> channel0 = std::make_shared<Text>(0,450,15,"Custom Channel:");
		guiMan.AddElement(channel0);
		std::shared_ptr<Text> chl0Name = std::make_shared<Text>(20,435,15,"Name: ");
		guiMan.AddElement(chl0Name);
		std::shared_ptr<TextInput> chl0NameInput = std::make_shared<TextInput>(75,435,200,15,synth.channels[0]->name,[&](TextInput* t){
			sndMan.start = false;
			if (synth.loadChannel(t->text->text,0) == -1)
				return;
			guiMan.GetElement<TextInput>(100)->SetText(to_string(synth.channels[0]->delayTime));
			guiMan.GetElement<TextInput>(101)->SetText(to_string(synth.channels[0]->attackTime));
			guiMan.GetElement<TextInput>(102)->SetText(to_string(synth.channels[0]->holdTime));
			guiMan.GetElement<TextInput>(103)->SetText(to_string(synth.channels[0]->decayTime));
			guiMan.GetElement<TextInput>(104)->SetText(to_string(synth.channels[0]->releaseTime));
			guiMan.GetElement<TextInput>(105)->SetText(to_string(synth.channels[0]->startVol));
			guiMan.GetElement<TextInput>(106)->SetText(to_string(synth.channels[0]->sustainVol));
			guiMan.GetElement<TextInput>(107)->SetText(to_string(synth.channels[0]->vibratoFreq));
			guiMan.GetElement<TextInput>(105)->SetText(to_string(synth.channels[0]->vibratoVol));
			sndMan.start = true;
		});

		chl0NameInput->SetStateColors(enabledColor,disabledColor);
		guiMan.AddElement(chl0NameInput,109);
		string timeLabels = "Delay Time: " +
										string("\nAttack Time: ") +
										string("\nHold Time: ") +
										string("\nDecay Time: ") +
										string("\nRelease Time: ") + 
										string("\n\nStart Volume: ") +
										string("\nSustain Volume: ") +
										string("\n\nVibrato Freq: ") +
										string("\nVibrato Volume: ");

		std::shared_ptr<Text> ch0NoteValues = std::make_shared<Text>(20,405,15,timeLabels);
		guiMan.AddElement(ch0NoteValues);	

		int baseY = 405;
		int yOff = 18;

		std::shared_ptr<TextInput> ch0delayTime = std::make_shared<TextInput>(150,baseY-(yOff*0),100,15,to_string(synth.channels[0]->delayTime),[&](TextInput* t){
			synth.channels[0]->delayTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0delayTime->SetStateColors(enabledColor,disabledColor);
		ch0delayTime->inputMask = "0123456789.";
		guiMan.AddElement(ch0delayTime,100);

		std::shared_ptr<TextInput> ch0attackTime = std::make_shared<TextInput>(150,baseY-(yOff*1),100,15,to_string(synth.channels[0]->attackTime),[&](TextInput* t){
			synth.channels[0]->attackTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0attackTime->SetStateColors(enabledColor,disabledColor);
		ch0attackTime->inputMask = "0123456789.";
		guiMan.AddElement(ch0attackTime,101);

		std::shared_ptr<TextInput> ch0holdTime = std::make_shared<TextInput>(150,baseY-(yOff*2),100,15,to_string(synth.channels[0]->holdTime),[&](TextInput* t){
			synth.channels[0]->holdTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0holdTime->SetStateColors(enabledColor,disabledColor);
		ch0holdTime->inputMask = "0123456789.";
		guiMan.AddElement(ch0holdTime,102);

		std::shared_ptr<TextInput> ch0decayTime = std::make_shared<TextInput>(150,baseY-(yOff*3),100,15,to_string(synth.channels[0]->decayTime),[&](TextInput* t){
			synth.channels[0]->decayTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0decayTime->SetStateColors(enabledColor,disabledColor);
		ch0decayTime->inputMask = "0123456789.";
		guiMan.AddElement(ch0decayTime,103);

		std::shared_ptr<TextInput> ch0releaseTime = std::make_shared<TextInput>(150,baseY-(yOff*4),100,15,to_string(synth.channels[0]->releaseTime),[&](TextInput* t){
			synth.channels[0]->releaseTime = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0releaseTime->SetStateColors(enabledColor,disabledColor);
		ch0releaseTime->inputMask = "0123456789.";
		guiMan.AddElement(ch0releaseTime,104);

		std::shared_ptr<TextInput> ch0startVol = std::make_shared<TextInput>(150,baseY-(yOff*6),100,15,to_string(synth.channels[0]->startVol),[&](TextInput* t){
			synth.channels[0]->startVol = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0startVol->SetStateColors(enabledColor,disabledColor);
		ch0startVol->inputMask = "0123456789.";
		guiMan.AddElement(ch0startVol,105);

		std::shared_ptr<TextInput> ch0sustainVol = std::make_shared<TextInput>(150,baseY-(yOff*7),100,15,to_string(synth.channels[0]->sustainVol),[&](TextInput* t){
			synth.channels[0]->sustainVol = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0sustainVol->SetStateColors(enabledColor,disabledColor);
		ch0sustainVol->inputMask = "0123456789.";
		guiMan.AddElement(ch0sustainVol,106);

		std::shared_ptr<TextInput> ch0vibratoFreq = std::make_shared<TextInput>(150,baseY-(yOff*9),100,15,to_string(synth.channels[0]->vibratoFreq),[&](TextInput* t){
			synth.channels[0]->vibratoFreq = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0vibratoFreq->SetStateColors(enabledColor,disabledColor);
		ch0vibratoFreq->inputMask = "0123456789.";
		guiMan.AddElement(ch0vibratoFreq,107);
		
		std::shared_ptr<TextInput> ch0vibratoVol = std::make_shared<TextInput>(150,baseY-(yOff*10),100,15,to_string(synth.channels[0]->vibratoVol),[&](TextInput* t){
			synth.channels[0]->vibratoVol = StringToDouble(&t->text->text);
			synth.channels[0]->updateNoteValues();
		});
		ch0vibratoVol->SetStateColors(enabledColor,disabledColor);
		ch0vibratoVol->inputMask = "0123456789.";
		guiMan.AddElement(ch0vibratoVol,108);

		std::shared_ptr<Text> channel1 = std::make_shared<Text>(270,450,15,"Channel 1:");
		guiMan.AddElement(channel1);
		std::shared_ptr<Text> ch1Name = std::make_shared<Text>(290,435,15,"Name: ");
		guiMan.AddElement(ch1Name);
		
		std::shared_ptr<TextInput> ch1NameInput = std::make_shared<TextInput>(345,435,200,15,"Yamaha Grand Piano",[&](TextInput* t){
			playMidi = false;
			midiMan.reset();
			sndMan.start = false;
			synth.loadChannel(t->text->text,1);
			sndMan.start = true;
		});
		ch1NameInput->SetStateColors(enabledColor,disabledColor);
		guiMan.AddElement(ch1NameInput,110);

		string midiPath = "Music/bems1.mid";
		std::shared_ptr<Text> midiName = std::make_shared<Text>(0,200,15,"Name: ");
		guiMan.AddElement(midiName);

		std::shared_ptr<TextInput> midiNameInput = std::make_shared<TextInput>(55,200,150,15,midiPath,[&](TextInput* t){
			playMidi = false;
			sndMan.start = false;
			midiMan.LoadFile(t->text->text);
			for (int i=0;i<=midiMan.numChannels;i++){
				synth.newChannel("Sine");
			}
			sndMan.start = true;
		});
		midiNameInput->SetStateColors(enabledColor,disabledColor);
		guiMan.AddElement(midiNameInput,111);

		std::shared_ptr<Button> midiPlayBtn = std::make_shared<Button>(280,200,40,15,MidiControl);
		midiPlayBtn->text->SetText("Play");
		midiPlayBtn->SetBackgroundColor(0.8,0.8,0.8);
		guiMan.AddElement(midiPlayBtn,200);

		std::shared_ptr<Button> midiResetBtn = std::make_shared<Button>(330,200,45,15,MidiControl);
		midiResetBtn->text->SetText("Reset");
		midiResetBtn->SetBackgroundColor(0.8,0.8,0.8);
		guiMan.AddElement(midiResetBtn,201);

		midiMan.LoadFile(midiPath);
		numChannels += midiMan.numChannels;
		
		/*
		synth.newChannel("Strings");
		synth.newChannel("Strings");
		synth.newChannel("Synth Brass 2");
		synth.newChannel("French Horns");
		synth.newChannel("Trombone");
		synth.newChannel("Strings");
		synth.newChannel("Clarinet");
		synth.newChannel("Timpani");
		synth.newChannel("Orchestra Hit");
		synth.newChannel("Orchestra");
		*/

		for (int i=0;i<midiMan.numChannels;i++){
			synth.newChannel(ch1NameInput->text->text);
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
					keys[i]->SetColor(1,1,1,1.0);
					whiteCount++;
					break;
				default:
					if (i == 6 || i == 13)
						xOffset += 45;
					keys[i] = new Panel(46*blackCount+35+xOffset,50,20,50);
					keys[i]->SetColor(0,0,0,1.0);
					blackCount++;
					break;
				}		
		}

		
		sndMan.start = true;
	}

	void OnUpdate(double dTime) {
		guiMan.DrawElements();
		
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
						keys[i]->SetColor(0.8,0.8,0.8,1.0);
					}
					else
						keys[i]->SetColor(1,1,1,1);
					keys[i]->Draw();
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
						keys[i]->SetColor(0.6,0.6,0.6,1.0);
					else
						keys[i]->SetColor(0,0,0,1);
					keys[i]->Draw();
					break;
				}		
		}
	}

	void OnShutdown(){

	}

	void OnEvent(Event &ev){
		guiMan.HandleEvent(ev);
		switch(ev.GetType()){
			case Event::Key:
				{
					int keyCode = static_cast<KeyEvent*>(&ev)->GetKeyCode();
					int keyState = static_cast<KeyEvent*>(&ev)->GetKeyState();
					bool noteOn = !keyState==GLFW_RELEASE;
					int noteOffset = -1;
					switch(keyCode){
						case GLFW_KEY_Z: noteOffset = 0;break;
						case GLFW_KEY_S: noteOffset = 1;break;
						case GLFW_KEY_X: noteOffset = 2;break; 
						case GLFW_KEY_D: noteOffset = 3;break;
						case GLFW_KEY_C: noteOffset = 4;break;
						case GLFW_KEY_V: noteOffset = 5;break;
						case GLFW_KEY_G: noteOffset = 6;break;
						case GLFW_KEY_B: noteOffset = 7;break;
						case GLFW_KEY_H: noteOffset = 8;break;
						case GLFW_KEY_N: noteOffset = 9;break;
						case GLFW_KEY_J: noteOffset = 10;break;
						case GLFW_KEY_M: noteOffset = 11;break;
						case GLFW_KEY_COMMA: noteOffset = 12;break;
						case GLFW_KEY_L: noteOffset = 13;break;
						case GLFW_KEY_PERIOD: noteOffset = 14;break;
						case GLFW_KEY_SEMICOLON: noteOffset = 15;break;
						case GLFW_KEY_SLASH: noteOffset = 16;break;
										 
					}
					if (noteOffset != -1){	
						synth.channels[0]->notes[60+noteOffset].setNoteState(noteOn);
						keyOn[noteOffset] = noteOn;
					}				
					break;
				}
			case Event::MouseButton:
				{
					MouseButtonEvent::ButtonType btn = static_cast<MouseButtonEvent*>(&ev)->GetButtonType();
					MouseButtonEvent::ButtonState state = static_cast<MouseButtonEvent*>(&ev)->GetButtonState();
					double xPos = static_cast<MouseButtonEvent*>(&ev)->GetMouseX();
					double yPos = static_cast<MouseButtonEvent*>(&ev)->GetMouseY();
					if (btn == MouseButtonEvent::ButtonType::Left){
						for (int i=0;i<17;i++){
							switch(i){
								case 1:
								case 3:
								case 6:
								case 8:
								case 10:
								case 13:
								case 15:
									if (keys[i]->CheckBoundingBox(xPos,yPos)){
										keySelected[i] = true;
										// mouse button
										if(state == MouseButtonEvent::ButtonState::Pressed){
											synth.channels[0]->notes[i+60].setNoteState(true);
										}
										else{
											synth.channels[0]->notes[i+60].setNoteState(false);
										}
									}
									else {
										if (!keyOn[i]){
											keySelected[i] = false;
											synth.channels[0]->notes[i+60].setNoteState(false);
										}
									}
									break;
								default:
									if (keys[i]->CheckBoundingBox(xPos,yPos)){
										int index = i-1;
										if (index < 0)
											index = 16;
										if (!(keys[(i+1)%16]->CheckBoundingBox(xPos,yPos)) && !(keys[index]->CheckBoundingBox(xPos,yPos))){
											keySelected[i] = true;
											if(state == MouseButtonEvent::ButtonState::Pressed){
												synth.channels[0]->notes[i+60].setNoteState(true);
												
											}
											else{
												synth.channels[0]->notes[i+60].setNoteState(false);
											}
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
					break;
				}
		}
	}
};


int main() {
	
	GraphicsEngine::Init();

	GraphicsEngine::AddWindow(new MainWindow(SCREENWIDTH,SCREENHEIGHT,"Synth"));
	GraphicsEngine::Run();

	return 0;
}
