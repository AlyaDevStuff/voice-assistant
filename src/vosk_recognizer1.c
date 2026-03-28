#include "vosk_recognizer.h"
#include "vosk_api.h"
#include <stdio.h>
#include <string.h>

int vosk_init(VoskContext *ctx, const char *modelPath) {
    printf("📦 Загрузка модели: %s\n", modelPath);
    
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
    
    printf("✅ Vosk инициализирован\n");
    return 0;
}

void vosk_cleanup(VoskContext *ctx) {
    if (ctx->recognizer) {
        vosk_recognizer_free((VoskRecognizer*)ctx->recognizer);
        ctx->recognizer = NULL;
    }
    if (ctx->model) {
        vosk_model_free((VoskModel*)ctx->model);
        ctx->model = NULL;
    }
    printf("🧹 Vosk очищен\n");
}

bool vosk_recognize(VoskContext *ctx, const short *audioData, 
                   int numSamples, char *result, int resultSize) {
    
    // Отправляем аудио в распознаватель
    vosk_recognizer_accept_waveform_s((VoskRecognizer*)ctx->recognizer, 
                                      audioData, numSamples);
    
    // Получаем результат
    const char *voskResult = vosk_recognizer_result((VoskRecognizer*)ctx->recognizer);
    
    if (!voskResult) {
        result[0] = '\0';
        return false;
    }
    
    // Парсим JSON: {"text":"привет"} (без пробелов!)
    const char *textStart = strstr(voskResult, "\"text\":\"");
    if (!textStart) {
        // Пробуем с пробелами: {"text": "привет"}
        textStart = strstr(voskResult, "\"text\" : \"");
        if (textStart) {
            textStart += 9; // Пропускаем '"text" : "'
        }
    } else {
        textStart += 8; // Пропускаем '"text":"'
    }
    
    if (textStart) {
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
