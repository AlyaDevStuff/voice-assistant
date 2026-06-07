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
    else if (strstr(text, "Как дела")) {
        printf("Отлично!\n");
        system("say 'Отлично!'");
    }
    else if (strstr(text, "время") || strstr(text, "времени") || strstr(text, "Который час") || strstr(text, "Сколько сейчас")) {
        system("say 'Сейчас' && date '+ %H:%M'");
    }
    else if (strstr(text, "браузер") || strstr(text, "сафари")) {
        printf("Открываю Safari\n");
        system("say 'Открываю Safari'");
        system("open -a Safari");
    }
    else if (strstr(text, "почта") || strstr(text, "письма") || strstr(text, "почту")) {
        printf("Открываю почту\n");
        system("say 'Открываю почту'");
        system("open -a Mail");
    }
    else if (strstr(text, "Выключи компьютер")) {
        system("say 'Выключаю'");
        system("osascript -e 'tell app \"System Events\" to shut down'");
    }
    else if (strstr(text, "Перезагрузи")) {
        system("say 'Перезагружаю'");
        system("osascript -e 'tell app \"System Events\" to restart'");
    }
    else if (strstr(text, "спящий режим") || strstr(text, "сон") || 
             strstr(text, "засыпай") || strstr(text, "Иди спать")) {
        printf("Спокойной ночи\n");
        system("say 'Спокойной ночи'");
        system("pmset sleepnow");
    }
    else if (strstr(text, "скриншот") || strstr(text, "снимок экрана")) {
        printf("Сейчас\n");
        system("say 'Сейчас'");
        system("screencapture ~/Desktop/screenshot_$(date +%Y%m%d_%H%M%S).png");
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
            if (strlen(text) > 0) {
                process_command(text);
                
                // ====== КОМАНДА "ПОКА" — КОРРЕКТНОЕ ЗАВЕРШЕНИЕ ======
                if (strstr(text, "пока") || strstr(text, "Пока") || 
                    strstr(text, "до свидания") || strstr(text, "До свидания") ||
                    strstr(text, "стоп") || strstr(text, "выход")) {
                    printf("Завершение работы...\n");
                    system("say 'До свидания'");
                    audio_buffer_free(&audio);
                    break; // Выход из while(1)
                }
                // =====================================================
            }
        }
        
        audio_buffer_free(&audio);
        printf("\n");
    }
    
    // ====== ОЧИСТКА ПАМЯТИ ПОСЛЕ ВЫХОДА ИЗ ЦИКЛА ======
    printf("Освобождение ресурсов...\n");
    whisper_free(ctx);
    audio_terminate();
    printf("Голосовой ассистент завершён.\n");
    // =====================================================
    
    return 0;
}