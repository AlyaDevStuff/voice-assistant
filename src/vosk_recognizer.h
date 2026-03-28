#ifndef VOSK_RECOGNIZER_H
#define VOSK_RECOGNIZER_H

#include <stdbool.h>

typedef struct {
    void *model;        // VoskModel*
    void *recognizer;   // VoskRecognizer*
} VoskContext;

int vosk_init(VoskContext *ctx, const char *modelPath);
void vosk_cleanup(VoskContext *ctx);
bool vosk_recognize(VoskContext *ctx, const short *audioData, int numSamples, char *result, int resultSize);

#endif