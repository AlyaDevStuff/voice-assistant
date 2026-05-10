cmake_minimum_required(VERSION 3.16)
project(voice_assistant VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Находим PortAudio через pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# Whisper.cpp как подпроект
set(GGML_METAL ON CACHE BOOL "Enable Metal" FORCE)
set(GGML_METAL_EMBED_LIBRARY ON CACHE BOOL "Embed Metal" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Static" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/whisper.cpp)

# Источники
set(SOURCES
    src/main.c
    src/audio_capture.c
)

# Заголовки
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${PORTAUDIO_INCLUDE_DIRS}
    deps/whisper.cpp/include
    deps/whisper.cpp/ggml/include
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Линковка
target_link_libraries(${PROJECT_NAME}
    whisper
    ${PORTAUDIO_LIBRARIES}
    pthread
    m
)

# Metal для macOS
if(APPLE)
    find_library(FOUNDATION Foundation)
    find_library(METAL Metal)
    find_library(METALKIT MetalKit)
    target_link_libraries(${PROJECT_NAME} 
        ${FOUNDATION}
        ${METAL}
        ${METALKIT}
    )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE 
    -Wall -Wextra -O2
    ${PORTAUDIO_CFLAGS_OTHER}
)

target_link_directories(${PROJECT_NAME} PRIVATE ${PORTAUDIO_LIBRARY_DIRS})
