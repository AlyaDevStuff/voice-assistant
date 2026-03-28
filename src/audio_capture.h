#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <stddef.h>

// Конфигурация аудио
#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 4000
#define NUM_CHANNELS 1

// Структура для записанных данных
typedef struct {
    float *samples;
    int numSamples;
    int capacity;
} AudioBuffer;

// Функции
int audio_init(void);
void audio_terminate(void);
int audio_record(AudioBuffer *buffer, int durationSec);
void audio_buffer_free(AudioBuffer *buffer);
void audio_float_to_short(const float *input, short *output, int numSamples);

#endif