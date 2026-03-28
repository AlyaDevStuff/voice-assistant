#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_capture.h"
#include "vosk_recognizer.h"

void process_command(const char *text) {
    printf("📝 Обработка команды: \"%s\"\n", text);
    
    if (strstr(text, "привет")) {
        printf("👋 Привет!\n");
        system("say 'Привет!'");
    }
    else if (strstr(text, "как дела")) {
        printf("Отлично. Я готов к работе\n");
        system("say 'Отлично. Я готов к работе'");
    }
    else if (strlen(text) > 0) {
        printf("❓ Неизвестная команда: %s\n", text);
    }
    else {
        printf("⚠️ Пустая команда\n");
    }
}

int main() {
    printf("🚀 Voice Assistant v0.1\n");
    printf("========================\n\n");
    
    if (audio_init() != 0) {
        fprintf(stderr, "❌ Ошибка инициализации аудио\n");
        return 1;
    }
    printf("✅ Аудио инициализировано\n");
    
    VoskContext vosk;
    if (vosk_init(&vosk, "models/vosk-model-small-ru-0.22") != 0) {
        audio_terminate();
        return 1;
    }
    
    printf("\n🎙️ Готов! Нажми Ctrl+C для выхода\n\n");
    
    int cycle = 0;
    while (1) {
        cycle++;
        printf("=== Цикл %d ===\n", cycle);
        
        AudioBuffer audio = {0};
        
        printf("🎤 Запись 4 секунд...\n");
        if (audio_record(&audio, 5) != 0) {
            break;
        }
        
        printf("📊 Записано: %d сэмплов\n", audio.numSamples);
        
        if (audio.numSamples == 0) {
            printf("⚠️ Пустая запись\n");
            audio_buffer_free(&audio);
            continue;
        }
        
        // Конвертация
        short *audioShort = (short*)malloc(audio.numSamples * sizeof(short));
        if (!audioShort) {
            printf("❌ Ошибка malloc\n");
            audio_buffer_free(&audio);
            continue;
        }
        
        audio_float_to_short(audio.samples, audioShort, audio.numSamples);
        printf("🔧 Конвертировано в int16\n");
        
        // Распознавание
        char recognizedText[256] = {0};
        printf("🧠 Вызываю vosk_recognize...\n");
        
        bool success = vosk_recognize(&vosk, audioShort, audio.numSamples, 
                                      recognizedText, sizeof(recognizedText));
        
        printf("✅ vosk_recognize вернул: %s\n", success ? "true" : "false");
        printf("📝 Текст: '%s'\n", recognizedText);
        
        // Обработка в любом случае (даже если пусто)
        if (strlen(recognizedText) > 0) {
            process_command(recognizedText);
        } else {
            printf("😶 Пустой результат\n");
        }
        
        free(audioShort);
        audio_buffer_free(&audio);
        printf("\n");
    }
    
    vosk_cleanup(&vosk);
    audio_terminate();
    printf("\n👋 До свидания!\n");
    return 0;
}
