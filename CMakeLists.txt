# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

# Initialize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Project name
project(simple-gamepad-proto C CXX ASM)

# C++ 標準の設定
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pico SDK の初期化
pico_sdk_init()

# ソースファイルを追加
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Pico 標準ライブラリをリンク
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
)

# USB 出力を有効化（デバッグ用）
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# UF2 ファイル等の生成
pico_add_extra_outputs(${PROJECT_NAME})
