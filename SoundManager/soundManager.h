#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include <pulse/pulseaudio.h>
#include <fstream>

class SoundManager {
	public:
		static pa_threaded_mainloop *s;
		static pa_context *context;
		static pa_mainloop_api *mainloop_api;
		static pa_stream *stream;

		static int bufSize;
		static uint8_t *soundBuf;
		static int outBufIndex;
		int sBufIndex = 0;

		uint8_t output = 0;

		static int samplesNeeded;
		static bool running;

		static unsigned int sampleRate;
		static bool start;

		SoundManager(int);
		~SoundManager();

		static uint32_t (*UserFunc)(double);

		void clock();

		static void quit();

		static void stream_state_callback(pa_stream *s, void *userdata);
		static void context_state_callback(pa_context *c, void *userdata);
		static void stream_write_callback(pa_stream *s, size_t length, void *userdata);
		static void exit_signal_callback(pa_mainloop_api*m, pa_signal_event *e, int sig, void *userdata);
};

#endif
