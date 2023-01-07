#pragma once
#include <stdint.h>
#include <stdlib.h>

#include <portaudio.h>
#include <emu2149.h>

#define SAMPLE_RATE         (44100)


class ScopedPaHandler
{
public:
    ScopedPaHandler() : _result(Pa_Initialize()) {
    }

    ~ScopedPaHandler() {
        if (_result == paNoError) {
            Pa_Terminate();
        }
    }

    PaError result() const { 
        return _result; 
    }

private:
    PaError _result;
};


class PSGEmulator
{
    ScopedPaHandler paInit;
    PaStream*       stream;

    PSG*            psg;
public:
    PSGEmulator();
    virtual ~PSGEmulator();

    bool open();
    bool close();

    bool stop();
    bool play();

    bool write( u_int8_t reg, u_int8_t val );

private:
    int paCallbackMethod(const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags);

    // This routine will be called by the PortAudio engine when audio is needed.
    // It may called at interrupt level on some machines so don't do anything
    // that could mess up the system like calling malloc() or free().
    static int paCallback( const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData );


    void paStreamFinishedMethod();

    // This routine is called by portaudio when playback is done.
    static void paStreamFinished(void* userData);

};

