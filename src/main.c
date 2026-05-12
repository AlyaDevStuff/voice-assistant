#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "audio_capture.h"
#include "whisper.h"

// ============ КОМАНДЫ ============

typedef struct {
    const char *keywords[4];   // Варианты названий
    const char *action;         // Команда macOS
    const char *response;       // Что сказать (опционально)
} Command;

Command commands[] = {
    // Почта
    {{"почта", "mail", "gmail", NULL}, 
     "open -a Mail", 
     "Открываю почту"},
    
    // Браузеры
    {{"браузер", "chrome", "хром", NULL}, 
     "open -a \"Google Chrome\"", 
     "Открываю браузер"},
    
    {{"сафари", "safari", NULL}, 
     "open -a Safari", 
     "Открываю Safari"},
    
    // Мессенджеры
    {{"телеграм", "telegram", "тг", NULL}, 
     "open -a Telegram", 
     "Открываю телеграм"},
    
    {{"вотсап", "whatsapp", NULL}, 
     "open -a WhatsApp", 
     "Открываю WhatsApp"},
    
    // Разработка
    {{"терминал", "консоль", NULL}, 
     "open -a Terminal", 
     "Открываю терминал"},
    
    {{"код", "vscode", NULL}, 
     "open -a \"Visual Studio Code\"", 
     "Открываю код"},
    
    // Медиа
    {{"музыка", "spotify", NULL}, 
     "open -a Spotify", 
     "Включаю музыку"},
    
    {{"ютуб", "youtube", NULL}, 
     "open -a \"Google Chrome\" \"https://youtube.com\"", 
     "Открываю YouTube"},
    
    // Система
    {{"настройки", "системные настройки", NULL}, 
     "open -a \"System Settings\"", 
     "Открываю настройки"},
    
    {NULL, NULL, NULL}
};

// ============ РАБОТА С ТЕКСТОМ ============

// Простой lowercase для UTF-8 (русский + английский)
void to_lower(char *str) {
    unsigned char *p = (unsigned char *)str;
    while (*p) {
        // Русские заглавные А-Я (кроме Ё)
        if (p[0] == 0xD0 && p[1] >= 0x90 && p[1] <= 0xAF) {
            p[1] += 0x20;  // А→а, Б→б, ...
            p += 2;
        }
        // Ё → ё
        else if (p[0] == 0xD0 && p[1] == 0x81) {
            p[0] = 0xD1; p[1] = 0x91;
            p += 2;
        }
        // Английские A-Z
        else if (*p >= 'A' && *p <= 'Z') {
            *p += ('a' - 'A');
            p++;
        }
        else {
            p++;
        }
    }
}

// Проверяет, есть ли слово в тексте
int word_in_text(const char *text, const char *word) {
    return strstr(text, word) != NULL;
}

// ============ ОБРАБОТКА КОМАНД ============

void process_command(const char *raw_text) {
    // Копируем и приводим к нижнему регистру
    char text[512];
    strncpy(text, raw_text, sizeof(text) - 1);
    text[sizeof(text) - 1] = '\0';
    to_lower(text);
    
    printf("Обработка: \"%s\"\n", text);
    
    // 1. Проверяем, обращались ли к Юре
    if (!word_in_text(text, "юр")) {
        printf("  → Не услышал 'Юр', игнорирую\n");
        return;
    }
    printf("  ✅ Обращение к Юре найдено\n");
    
    // 2. Ищем команду
    for (int i = 0; commands[i].keywords[0] != NULL; i++) {
        for (int k = 0; commands[i].keywords[k] != NULL; k++) {
            if (word_in_text(text, commands[i].keywords[k])) {
                printf("  🎯 Команда: %s\n", commands[i].response);
                printf("  ⚡ Выполняю: %s\n", commands[i].action);
                
                // Говорим ответ
                char say_cmd[256];
                snprintf(say_cmd, sizeof(say_cmd), "say '%s'", commands[i].response);
                system(say_cmd);
                
                // Выполняем действие
                int result = system(commands[i].action);
                if (result != 0) {
                    printf("  ⚠️ Ошибка выполнения: %d\n", result);
                    system("say 'Не получилось открыть'");
                }
                return;
            }
        }
    }
    
    // 3. Команда не найдена
    printf("  ❓ Не понял команду. Попробуй: 'Юр, открой почту'\n");
    system("say 'Не понял команду'");
}

// ============ ГЛАВНАЯ ============

int main(int argc, char** argv) {
    const char* model_path = (argc > 1) ? argv[1] : "models/ggml-base.ru.bin";
    
    printf("╔═══════════════════════════════════════╗\n");
    printf("║     🤖 Юра — Голосовой помощник      ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    printf("Модель: %s\n\n", model_path);
    
    // Инициализация аудио
    if (audio_init() != 0) {
        fprintf(stderr, "❌ Ошибка инициализации аудио\n");
        return 1;
    }
    
    // Загрузка модели Whisper
    struct whisper_context_params cparams = whisper_context_default_params();
    struct whisper_context* ctx = whisper_init_from_file_with_params(model_path, cparams);
    if (!ctx) {
        fprintf(stderr, "❌ Ошибка загрузки модели\n");
        audio_terminate();
        return 1;
    }
    
    // Настройки распознавания
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.print_progress = false;
    wparams.print_special = false;
    wparams.print_realtime = false;
    wparams.print_timestamps = false;
    wparams.translate = false;
    wparams.language = "ru";
    wparams.n_threads = 4;
    
    printf("Готов! Скажи: 'Юр, открой почту'\n");
    printf("Для выхода: Ctrl+C\n\n");
    
    // Главный цикл
    while (1) {
        AudioBuffer audio = {0};
        
        printf("🎤 Запись 5 секунд...\n");
        if (audio_record(&audio, 5) != 0) {
            printf("❌ Ошибка записи\n");
            break;
        }
        
        if (audio.numSamples == 0) {
            audio_buffer_free(&audio);
            continue;
        }
        
        printf("🧠 Распознаю...\n");
        if (whisper_full(ctx, wparams, audio.samples, audio.numSamples) == 0) {
            int n = whisper_full_n_segments(ctx);
            char text[512] = {0};
            int offset = 0;
            
            // Собираем текст из сегментов
            for (int i = 0; i < n && offset < 500; i++) {
                const char* seg = whisper_full_get_segment_text(ctx, i);
                int len = strlen(seg);
                
                // Пропускаем артефакты вроде [музыка], [шум]
                if (len > 0 && seg[0] != '[') {
                    if (offset > 0) text[offset++] = ' ';
                    if (offset + len < 500) {
                        strcpy(text + offset, seg);
                        offset += len;
                    }
                }
            }
            
            printf("📝 Распознано: \"%s\"\n", text);
            
            if (strlen(text) > 0) {
                process_command(text);
            } else {
                printf("  ⚠️ Ничего не распознано\n");
            }
        } else {
            printf("❌ Ошибка распознавания\n");
        }
        
        audio_buffer_free(&audio);
        printf("\n");
    }
    
    // Очистка
    whisper_free(ctx);
    audio_terminate();
    printf("\n👋 До свидания!\n");
    return 0;
}