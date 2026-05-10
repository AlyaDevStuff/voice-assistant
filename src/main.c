#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "audio_capture.h"
#include "whisper.h"

void process_command(const char *text) {
    printf("Обработка: \"%s\"\n", text);
    if (strstr(text, "привет") || strstr(text, "Привет")) {
        printf("Привет!\n");
        system("say 'Привет!'");
    }
    else if (strstr(text, "как дела")) {
        printf("Отлично!\n");
        system("say 'Отлично!'");
    }
    else if (strlen(text) > 0) {
        printf("Неизвестно: %s\n", text);
    }
}

int main(int argc, char** argv) {
    const char* model_path = (argc > 1) ? argv[1] : "models/ggml-model.bin";
    printf("Voice Assistant (Whisper)\nМодель: %s\n\n", model_path);
    
    if (audio_init() != 0) {
        fprintf(stderr, "Ошибка аудио\n");
        return 1;
    }
    
    struct whisper_context_params cparams = whisper_context_default_params();
    struct whisper_context* ctx = whisper_init_from_file_with_params(model_path, cparams);
    if (!ctx) {
        fprintf(stderr, "Ошибка модели\n");
        audio_terminate();
        return 1;
    }
    
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.print_progress = false;
    wparams.print_special = false;
    wparams.print_realtime = false;
    wparams.print_timestamps = false;
    wparams.translate = false;
    wparams.language = "ru";
    wparams.n_threads = 4;
    
    printf("Готов! Говори (Ctrl+C для выхода)\n\n");
    
    while (1) {
        AudioBuffer audio = {0};
        printf("Запись 5 сек...\n");
        if (audio_record(&audio, 5) != 0) break;
        
        if (audio.numSamples == 0) {
            audio_buffer_free(&audio);
            continue;
        }
        
        // Данные уже в float [-1.0, 1.0] от PortAudio!
        // НЕ конвертируем, передаём напрямую в whisper
        printf("Распознаю...\n");
        if (whisper_full(ctx, wparams, audio.samples, audio.numSamples) == 0) {
            int n = whisper_full_n_segments(ctx);
            char text[512] = {0};
            int offset = 0;
            
            for (int i = 0; i < n && offset < 500; i++) {
                const char* seg = whisper_full_get_segment_text(ctx, i);
                int len = strlen(seg);
                if (len > 0 && seg[0] != '[') {
                    if (offset > 0) text[offset++] = ' ';
                    if (offset + len < 500) {
                        strcpy(text + offset, seg);
                        offset += len;
                    }
                }
            }
            
            printf("Распознано: \"%s\"\n", text);
            if (strlen(text) > 0) process_command(text);
        }
        
        audio_buffer_free(&audio);
        printf("\n");
    }
    
    whisper_free(ctx);
    audio_terminate();
    return 0;
}
