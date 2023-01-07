#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h> // for dirname()
#include <unistd.h>

#include "psg.h"


PSGEmulator::PSGEmulator()
{
    // Typical AY-3-8910 configuration
    psg = PSG_new(3579545/2, SAMPLE_RATE);
    PSG_setVolumeMode(psg, 2); // AY style
    PSG_reset(psg);

    PSG_writeReg(psg, 0x07, 0x3e );
    PSG_writeReg(psg, 0x08, 0x1f ); // PSG.A VOL
    PSG_writeReg(psg, 0x09, 0x1f ); // PSG.B VOL
    PSG_writeReg(psg, 0x0A, 0x1f ); // PSG.C VOL
}

PSGEmulator::~PSGEmulator()
{
    PSG_delete(psg);
    psg = NULL;
}

bool PSGEmulator::open()
{
    PaDeviceIndex index = Pa_GetDefaultOutputDevice();
    printf("Pa_GetDefaultOutputDevice:%d\r\n", index);

    if ( paInit.result() != paNoError)
    {
        printf("Error:paInit\n");
        return false;
    }

    PaStreamParameters outputParameters;

    outputParameters.device = index;
    if (outputParameters.device == paNoDevice)
    {
        return false;
    }

    const PaDeviceInfo *pInfo = Pa_GetDeviceInfo(index);
    if (pInfo != 0)
    {
        printf("Output device name: %s\n", pInfo->name);
    }

    outputParameters.channelCount = 2; // stereo output
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaError err = Pa_OpenStream(
        &stream,
        NULL, // no input
        &outputParameters,
        SAMPLE_RATE,
        paFramesPerBufferUnspecified,
        paClipOff, // we won't output out of range samples so don't bother clipping them
        &PSGEmulator::paCallback,
        this
    );

    if (err != paNoError)
    {
        // Failed to open stream to device !!!
        return false;
    }

    err = Pa_SetStreamFinishedCallback(stream, &PSGEmulator::paStreamFinished);

    if (err != paNoError)
    {
        Pa_CloseStream(stream);
        stream = 0;

        return false;
    }

    return true;
}


bool PSGEmulator::close()
{
    if (stream == 0)
    {
        return false;
    }

    PaError err = Pa_CloseStream(stream);
    stream = 0;



    return (err == paNoError);
}

bool PSGEmulator::write( u_int8_t reg, u_int8_t val )
{
//    printf("writeReg %02x, %02x\n", reg, val);
    PSG_writeReg(psg, reg, val);
    return true;
}


bool PSGEmulator::play()
{
    if (stream == 0)
    {
        return false;
    }


    PaError err = Pa_StartStream(stream);

    return (err == paNoError);
}

bool PSGEmulator::stop()
{
    if (stream == 0)
    {
        return false;
    }

    PaError err = Pa_StopStream(stream);

    return (err == paNoError);
}


int PSGEmulator::paCallbackMethod(const void *inputBuffer, void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo *timeInfo,
                                PaStreamCallbackFlags statusFlags)
{

    (void)timeInfo; // Prevent unused variable warnings.
    (void)statusFlags;
    (void)inputBuffer;

    int16_t* buf = (int16_t*)outputBuffer;
    for (int i=0;i<framesPerBuffer;++i) {
        int16_t out = PSG_calc(psg);
        buf[i*2+0] = out;   // L
        buf[i*2+1] = out;   // R
    }
    return paContinue;
}

// This routine will be called by the PortAudio engine when audio is needed.
// It may called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
int PSGEmulator::paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    // Here we cast userData to PSG* type so we can call the instance method paCallbackMethod, we can do that since
    //   we called Pa_OpenStream with 'this' for userData
    return ((PSGEmulator *)userData)->paCallbackMethod(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

void PSGEmulator::paStreamFinishedMethod()
{
}

//
// This routine is called by portaudio when playback is done.
//
void PSGEmulator::paStreamFinished(void *userData)
{
    return ((PSGEmulator *)userData)->paStreamFinishedMethod();
}

