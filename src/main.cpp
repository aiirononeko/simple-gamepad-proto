/**
 * @file main.cpp
 * @brief Raspberry Pi Pico メインエントリーポイント
 *
 * シンプルなLED点滅プログラム（サンプル）
 */

#include "pico/stdlib.h"
#include <cstdio>

// オンボードLEDのピン番号
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

int main() {
    // 標準入出力の初期化（USBシリアル）
    stdio_init_all();

    // LEDピンを出力として初期化
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    printf("Raspberry Pi Pico 起動完了!\n");

    // メインループ
    while (true) {
        // LED ON
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        printf("LED: ON\n");
        sleep_ms(500);

        // LED OFF
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        printf("LED: OFF\n");
        sleep_ms(500);
    }

    return 0;
}
