#include "soundManager.h"
#include <iostream>
#include <cmath>

using namespace std;

pa_stream *SoundManager::stream = NULL;
pa_threaded_mainloop *SoundManager::s = NULL;
pa_context *SoundManager::context = NULL;
pa_mainloop_api *SoundManager::mainloop_api = NULL;

unsigned int SoundManager::sampleRate = 0;
bool SoundManager::start = false;
int SoundManager::bufSize = 48000*2;
uint8_t *SoundManager::soundBuf = new uint8_t[SoundManager::bufSize];
int SoundManager::outBufIndex = 0;
int SoundManager::samplesNeeded = 0;
bool SoundManager::running = true;
uint32_t (*SoundManager::UserFunc)(double) = 0;

static double n = 0;

void SoundManager::stream_write_callback(pa_stream *s, size_t length, void *userdata){

	samplesNeeded = length;
	uint16_t buf[length];
	//cout << "--------" << length << endl;
	for (int i=0;i<length/2;i+=2){
		if (start){
			uint32_t sampleData = UserFunc(n);
			//Left
			buf[i] = (sampleData&0xFFFF0000)/0x10000;
			//Right			
			buf[i+1] = sampleData&0x0000FFFF;
			n += (1/((double)sampleRate));
		}
		else{
			buf[i] = 0;
			buf[i+1] = 0;
		}
	}

	
	//cout << outBufIndex << endl;
	pa_stream_write(s,buf,length,NULL,0,PA_SEEK_RELATIVE);
	
}

void SoundManager::stream_state_callback(pa_stream *s, void *userdata){
	cout << "Stream Test" << endl;
}	

void SoundManager::context_state_callback(pa_context *c, void *userdata){
	cout << "Context Test" << endl;
	if (pa_context_get_state(c) != PA_CONTEXT_READY)
		return;

	static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = sampleRate,
        .channels = 2
    };

		int bufSize1 = 8196;

		const pa_buffer_attr bufAttr = {
		.maxlength = (uint32_t)bufSize1,
		.tlength = (uint32_t)-1,
		.prebuf = (uint32_t)-1,
		.minreq = (uint32_t)-1,
		.fragsize = (uint32_t)-1		
	};
	
	
	
	stream = pa_stream_new(c,"Synth",&ss,NULL);
pa_stream_set_state_callback(stream,stream_state_callback,NULL);		  pa_stream_set_write_callback(stream,stream_write_callback,NULL);
	pa_stream_connect_playback(stream,NULL,&bufAttr,(pa_stream_flags_t)0,NULL,NULL);
}

SoundManager::SoundManager(int sRate){
	sampleRate = sRate;
	bufSize = sampleRate*2;
		
	int ret = 0;

	s = pa_threaded_mainloop_new();
	mainloop_api = pa_threaded_mainloop_get_api(s);
	context = pa_context_new(mainloop_api,"Test");
	
	pa_context_set_state_callback(context,context_state_callback,NULL);

	int r = pa_signal_init(mainloop_api);
	if (r == 0)
		 pa_signal_new(SIGINT, exit_signal_callback, NULL);
	
	pa_context_connect(context,NULL,(pa_context_flags_t)0,NULL);
	
	pa_threaded_mainloop_start(s);
}
	
void SoundManager::quit(){
	int ret = 0;
	running = false;
	mainloop_api->quit(mainloop_api,ret);
	pa_stream_disconnect(stream);
	pa_context_disconnect(context);
	pa_threaded_mainloop_stop(s);
	pa_threaded_mainloop_free(s);
}

void SoundManager::exit_signal_callback(pa_mainloop_api*m, pa_signal_event *e, int sig, void *userdata) {
	quit();
}

SoundManager::~SoundManager(){
	quit();
}

int max1 = 124;
	int count = 48000;
	int mask = 1;
	int j = 1;
	int duty = 0b00001111;

void SoundManager::clock(){
			
	soundBuf[sBufIndex] = (output);
	if (sBufIndex == bufSize-1){
		sBufIndex = 0;
	}
	else
		sBufIndex++;
	samplesNeeded--;
}
