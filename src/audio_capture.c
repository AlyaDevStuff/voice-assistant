#include "audio_capture.h"
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PaStream *g_stream = NULL;

typedef struct {
    float *buffer;
    int maxFrames;
    int frameIndex;
} RecordData;

static int recordCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    (void)outputBuffer;
    (void)timeInfo;
    (void)statusFlags;
    
    RecordData *data = (RecordData*)userData;
    const float *rptr = (const float*)inputBuffer;
    float *wptr = &data->buffer[data->frameIndex];
    unsigned long framesLeft = data->maxFrames - data->frameIndex;
    unsigned long framesToCalc = framesPerBuffer;
    
        if (inputBuffer != NULL && data->frameIndex % 16000 == 0) {  // Каждую секунду
        float max = 0;
        for (int i = 0; i < framesToCalc; i++) {
            float val = ((const float*)inputBuffer)[i];
            if (val > max) max = val;
            if (-val > max) max = -val;
        }
        printf("[AUDIO] Уровень звука: %.4f (должно быть > 0.01 при речи)\n", max);
    }
    
    if (framesLeft < framesToCalc) {
        framesToCalc = framesLeft;
    }
    
    if (inputBuffer == NULL) {
        for (unsigned long i = 0; i < framesToCalc; i++) {
            *wptr++ = 0.0f;
        }
    } else {
        for (unsigned long i = 0; i < framesToCalc; i++) {
            *wptr++ = *rptr++;
        }
    }
    
    data->frameIndex += framesToCalc;
    return (data->frameIndex >= data->maxFrames) ? paComplete : paContinue;
}

int audio_init(void) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }
    return 0;
}

void audio_terminate(void) {
    Pa_Terminate();
}

int audio_record(AudioBuffer *buffer, int durationSec) {
    PaStreamParameters inputParameters;
    RecordData data;
    int totalFrames = durationSec * SAMPLE_RATE;
    
    buffer->samples = (float*)malloc(totalFrames * sizeof(float));
    if (!buffer->samples) {
        fprintf(stderr, "Cannot allocate memory\n");
        return 1;
    }
    buffer->capacity = totalFrames;
    buffer->numSamples = 0;
    
    data.buffer = buffer->samples;
    data.maxFrames = totalFrames;
    data.frameIndex = 0;
    
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        free(buffer->samples);
        return 1;
    }
    
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = 
        Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    PaError err = Pa_OpenStream(&g_stream, &inputParameters, NULL,
                                SAMPLE_RATE, FRAMES_PER_BUFFER,
                                paClipOff, recordCallback, &data);
    if (err != paNoError) {
        fprintf(stderr, "Pa_OpenStream error: %s\n", Pa_GetErrorText(err));
        free(buffer->samples);
        return 1;
    }
    
    printf("🎤 Говорите %d секунд...\n", durationSec);
    
    err = Pa_StartStream(g_stream);
    if (err != paNoError) {
        fprintf(stderr, "Pa_StartStream error: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(g_stream);
        free(buffer->samples);
        return 1;
    }
    
    while ((err = Pa_IsStreamActive(g_stream)) == 1) {
        Pa_Sleep(100);
    }
    
    Pa_CloseStream(g_stream);
    g_stream = NULL;
    
    buffer->numSamples = data.frameIndex;
    printf("✅ Записано %d сэмплов\n", buffer->numSamples);
    
    return 0;
}

void audio_buffer_free(AudioBuffer *buffer) {
    if (buffer->samples) {
        free(buffer->samples);
        buffer->samples = NULL;
    }
    buffer->numSamples = 0;
    buffer->capacity = 0;
}

void audio_float_to_short(const float *input, short *output, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        float sample = input[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        output[i] = (short)(sample * 32767.0f);
    }
}