CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
INCLUDES = -I./include -I/opt/homebrew/include -I/usr/local/include

# Статическая библиотека
VOSK_LIB = ./lib/libvosk.a

LDFLAGS = -L/opt/homebrew/lib -L/usr/local/lib
# ВАЖНО: порядок! Сначала -lpthread, потом -lm
LIBS = -lportaudio -lpthread -lm

TARGET = voice_assistant
SRCDIR = src
OBJDIR = build

SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/audio_capture.c \
          $(SRCDIR)/vosk_recognizer.c

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

.PHONY: all clean run dirs check

all: check dirs $(TARGET)

dirs:
	@mkdir -p $(OBJDIR)

# ВАЖНО: VOSK_LIB после OBJECTS, но перед системными библиотеками!
$(TARGET): $(OBJECTS) $(VOSK_LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(VOSK_LIB) $(LIBS)
	@echo "✅ Сборка завершена"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

check:
	@echo "🔍 Проверка..."
	@test -f $(VOSK_LIB) || (echo "❌ Нет $(VOSK_LIB)" && exit 1)
	@ar -t $(VOSK_LIB) > /dev/null 2>&1 || (echo "❌ Библиотека повреждена" && exit 1)
	@echo "✅ Библиотека на месте"
