# 🎙️ Voice Assistant (Whisper.cpp)

Голосовой ассистент для macOS с распознаванием речи через Whisper.cpp на Apple Silicon (Metal GPU).

## ✨ Возможности

- 🎤 Запись аудио с микрофона
- 🧠 Распознавание русской речи в реальном времени
- ⚡ Ускорение на Apple M4 через Metal GPU
- 🎯 Выполнение голосовых команд

## 🗣️ Поддерживаемые команды

| Команда | Действие |
|---------|----------|
| "Привет" | Отвечает голосом "Привет!" |
| "Как дела" | Отвечает "Отлично!" |

## 📋 Требования

- macOS с Apple Silicon (M1/M2/M3/M4)
- [Homebrew](https://brew.sh)
- CMake 3.16+
- PortAudio

## 🚀 Быстрый старт

### 1. Установка зависимостей

```bash
brew install cmake portaudio pkg-config
```

### 2. Клонирование проекта

```bash
git clone https://github.com/AlyaDevStuff/voice-assistant.git
cd voice-assistant
```

### 3. Скачать whisper.cpp

Whisper.cpp не включён в репозиторий (слишком большой). Скачайте отдельно:

```bash
git clone --depth 1 https://github.com/ggerganov/whisper.cpp.git deps/whisper.cpp
```

### 4. Собрать whisper.cpp

```bash
cd deps/whisper.cpp
cmake -B build -DGGML_METAL=ON -DBUILD_EXAMPLES=OFF
cmake --build build --config Release -j4
cd ../..
```

### 5. Получить модель Whisper (~500MB)

**Вариант А: Скачать готовую (если интернет позволяет)**

```bash
./deps/whisper.cpp/models/download-ggml-model.sh small
cp deps/whisper.cpp/models/ggml-small.bin models/ggml-model.bin
```

**Вариант Б: Конвертировать из PyTorch (если HuggingFace недоступен)**

```bash
pip install openai-whisper
python3 -c "import whisper; m = whisper.load_model('small')"

# Нужен файл mel_filters.npz из установленного пакета
mkdir -p deps/whisper.cpp/whisper/assets
cp $(python3 -c "import whisper; import os; print(os.path.dirname(whisper.__file__))")/assets/mel_filters.npz \
   deps/whisper.cpp/whisper/assets/

# Конвертация
python3 deps/whisper.cpp/models/convert-pt-to-ggml.py \
  ~/.cache/whisper/small.pt \
  deps/whisper.cpp \
  models/
```

### 6. Собрать проект

```bash
mkdir build && cd build
cmake ..
cmake --build . -j4
```

### 7. Запуск

```bash
./voice_assistant ../models/ggml-model.bin
```

Когда увидите `Запись 5 сек...` — говорите команду в микрофон!

## 🏗️ Структура проекта

```
voice-assistant/
├── CMakeLists.txt          # Конфигурация сборки
├── src/
│   ├── main.c              # Главный цикл (Whisper + команды)
│   ├── audio_capture.c     # Запись с микрофона
│   ├── audio_capture.h     # Заголовки аудио
│   └── record_test.c       # Тестовая запись WAV
├── models/                 # Модели Whisper (не в Git)
├── deps/
│   └── whisper.cpp/        # Подмодуль whisper.cpp (не в Git)
└── build/                  # Сборка (генерируется CMake)
```

## ⚠️ Важно

| Что | Где | Примечание |
|-----|-----|------------|
| whisper.cpp | `deps/whisper.cpp` | Скачивается отдельно, не в Git |
| Модель Whisper | `models/ggml-model.bin` | ~500MB, не в Git |
| Сборка | `build/` | Генерируется CMake, не в Git |

## 🔧 Устранение неполадок

### whisper-cli не находит библиотеки

```bash
export DYLD_LIBRARY_PATH=deps/whisper.cpp/build/src:deps/whisper.cpp/build/ggml/src:$DYLD_LIBRARY_PATH
```

### Модель не скачивается с HuggingFace

Используйте Вариант Б (конвертация из PyTorch) в разделе 5.

### Нет звука или тихая запись

Проверьте настройки микрофона в macOS: **Системные настройки → Звук → Вход**
