#include "vosk_recognizer.h"
#include "vosk_api.h"
#include <stdio.h>
#include <string.h>

// ============================================
// ЗАГЛУШКА VOSK (временная, для тестирования)
// Удали эту секцию когда будет настоящий libvosk.dylib
// ============================================

static int stub_counter = 0;

// Простая реализация функций Vosk
VoskModel* vosk_model_new(const char* path) { 
    printf("[STUB] Загрузка модели: %s\n", path); 
    return (VoskModel*)0x1234; 
}

void vosk_model_free(VoskModel* m) { 
    (void)m;
    printf("[STUB] Освобождение модели\n"); 
}

VoskRecognizer* vosk_recognizer_new(VoskModel* m, float sample_rate) { 
    (void)m;
    (void)sample_rate;
    printf("[STUB] Создание распознавателя\n"); 
    return (VoskRecognizer*)0x5678; 
}

void vosk_recognizer_free(VoskRecognizer* r) { 
    (void)r;
    printf("[STUB] Освобождение распознавателя\n"); 
}

int vosk_recognizer_accept_waveform_s(VoskRecognizer* r, 
                                      const short* data, 
                                      int length) { 
    (void)r;
    (void)data;
    (void)length;
    return 1; 
}

const char* vosk_recognizer_result(VoskRecognizer* r) { 
    (void)r;
    stub_counter++;
    
    // Меняй эти команды для тестирования!
    // Они возвращаются по очереди (циклически)
    switch(stub_counter % 6) {
        case 1: return "{\"text\":\"привет\"}";
        case 2: return "{\"text\":\"как дела?\"}";
        default: return "{\"text\":\"привет\"}";
    }
}

const char* vosk_recognizer_partial_result(VoskRecognizer* r) { 
    (void)r;
    return "{\"partial\":\"\"}"; 
}

void vosk_recognizer_reset(VoskRecognizer* r) { 
    (void)r;
}

// ============================================
// ОСНОВНОЙ КОД (не трогай при замене заглушки)
// ============================================

int vosk_init(VoskContext *ctx, const char *modelPath) {
    printf("[VOSK] Инициализация...\n");
    
    VoskModel *model = vosk_model_new(modelPath);
    if (!model) {
        fprintf(stderr, "❌ Ошибка загрузки модели\n");
        return 1;
    }
    
    VoskRecognizer *recognizer = vosk_recognizer_new(model, 16000.0);
    if (!recognizer) {
        fprintf(stderr, "❌ Ошибка создания распознавателя\n");
        vosk_model_free(model);
        return 1;
    }
    
    ctx->model = model;
    ctx->recognizer = recognizer;
    
    printf("[VOSK] Готово\n");
    return 0;
}

void vosk_cleanup(VoskContext *ctx) {
    printf("[VOSK] Очистка...\n");
    
    if (ctx->recognizer) {
        vosk_recognizer_free((VoskRecognizer*)ctx->recognizer);
        ctx->recognizer = NULL;
    }
    if (ctx->model) {
        vosk_model_free((VoskModel*)ctx->model);
        ctx->model = NULL;
    }
    
    printf("[VOSK] Очищено\n");
}

bool vosk_recognize(VoskContext *ctx, 
                    const short *audioData, 
                    int numSamples, 
                    char *result, 
                    int resultSize) {
    
    // Отправляем аудио в распознаватель
    vosk_recognizer_accept_waveform_s((VoskRecognizer*)ctx->recognizer, 
                                      audioData, 
                                      numSamples);
    
    // Получаем результат
    const char *voskResult = vosk_recognizer_result((VoskRecognizer*)ctx->recognizer);
    
    if (!voskResult) {
        result[0] = '\0';
        return false;
    }
    
    // Парсим JSON: {"text":"команда"}
    // Пробуем без пробелов
    const char *textStart = strstr(voskResult, "\"text\":\"");
    int offset = 8;
    
    // Если не нашли, пробуем с пробелами
    if (!textStart) {
        textStart = strstr(voskResult, "\"text\" : \"");
        offset = 9;
    }
    
    if (textStart) {
        textStart += offset;
        const char *textEnd = strchr(textStart, '"');
        
        if (textEnd) {
            int len = textEnd - textStart;
            if (len >= resultSize) len = resultSize - 1;
            
            strncpy(result, textStart, len);
            result[len] = '\0';
            
            return len > 0;
        }
    }
    
    result[0] = '\0';
    return false;
}