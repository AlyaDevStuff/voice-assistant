#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_capture.h"

int main() {
    if (audio_init() != 0) {
        fprintf(stderr, "Ошибка аудио\n");
        return 1;
    }
    
    printf("Запись 5 секунд...\n");
    AudioBuffer audio = {0};
    if (audio_record(&audio, 5) != 0) {
        audio_terminate();
        return 1;
    }
    
    printf("Записано %d сэмплов\n", audio.numSamples);
    
    // Сохраняем в WAV файл в текущую папку
    const char* filename = "test_recording.wav";
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Ошибка создания файла %s\n", filename);
        audio_buffer_free(&audio);
        audio_terminate();
        return 1;
    }
    
    // WAV header
    int sampleRate = 16000;
    int numChannels = 1;
    int bitsPerSample = 16;
    int dataSize = audio.numSamples * sizeof(short);
    
    fwrite("RIFF", 1, 4, f);
    int chunkSize = 36 + dataSize;
    fwrite(&chunkSize, 4, 1, f);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    int subChunk1Size = 16;
    fwrite(&subChunk1Size, 4, 1, f);
    short audioFormat = 1;
    fwrite(&audioFormat, 2, 1, f);
    fwrite(&numChannels, 2, 1, f);
    fwrite(&sampleRate, 4, 1, f);
    int byteRate = sampleRate * numChannels * bitsPerSample / 8;
    fwrite(&byteRate, 4, 1, f);
    short blockAlign = numChannels * bitsPerSample / 8;
    fwrite(&blockAlign, 2, 1, f);
    fwrite(&bitsPerSample, 2, 1, f);
    fwrite("data", 1, 4, f);
    fwrite(&dataSize, 4, 1, f);
    
    // Конвертация float -> int16 и запись
    short* samples = malloc(audio.numSamples * sizeof(short));
    audio_float_to_short(audio.samples, samples, audio.numSamples);
    fwrite(samples, sizeof(short), audio.numSamples, f);
    
    free(samples);
    fclose(f);
    
    printf("Сохранено: %s (%d сэмплов)\n", filename, audio.numSamples);
    
    audio_buffer_free(&audio);
    audio_terminate();
    return 0;
}
