/**
 * @file main.cpp
 * @brief Raspberry Pi Pico ゲームパッドプロトタイプ
 *
 * GP14: Aボタン
 * GP15: Bボタン
 * GP4: I2C SDA (LCD)
 * GP5: I2C SCL (LCD)
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_i2c.h"
#include <cstdio>

// ボタンピンの定義
#define BUTTON_A_PIN 14  // GP14
#define BUTTON_B_PIN 15  // GP15

// I2Cピンの定義
#define I2C_SDA_PIN 4    // GP4
#define I2C_SCL_PIN 5    // GP5
#define I2C_BAUDRATE 100000  // 100kHz

// LCD I2Cアドレス (0x27 または 0x3F、モジュールにより異なる)
#define LCD_ADDR 0x27

int main() {
    // 標準入出力の初期化（USBシリアル）
    stdio_init_all();

    // Aボタン（GP14）を入力として初期化（内部プルアップ有効）
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    // Bボタン（GP15）を入力として初期化（内部プルアップ有効）
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // I2C初期化
    i2c_init(i2c0, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // LCD初期化
    LcdI2c lcd(i2c0, LCD_ADDR);
    lcd.init();
    lcd.clear();

    // 起動メッセージ表示
    lcd.setCursor(0, 0);
    lcd.print("Simple Gamepad");
    lcd.setCursor(0, 1);
    lcd.print("A:GP14  B:GP15");
    lcd.setCursor(0, 2);
    lcd.print("--------------------");
    lcd.setCursor(0, 3);
    lcd.print("A:OFF  B:OFF");

    printf("Raspberry Pi Pico ゲームパッド起動完了!\n");

    // 前回のボタン状態を保持
    bool prev_button_a = true;  // プルアップなので初期状態はHIGH
    bool prev_button_b = true;

    // メインループ
    while (true) {
        // ボタン状態を読み取り（プルアップなので押されると LOW）
        bool button_a = gpio_get(BUTTON_A_PIN);
        bool button_b = gpio_get(BUTTON_B_PIN);

        bool state_changed = false;

        // Aボタンの状態変化を検出
        if (button_a != prev_button_a) {
            if (!button_a) {
                printf("Aボタン: 押された\n");
            } else {
                printf("Aボタン: 離された\n");
            }
            prev_button_a = button_a;
            state_changed = true;
        }

        // Bボタンの状態変化を検出
        if (button_b != prev_button_b) {
            if (!button_b) {
                printf("Bボタン: 押された\n");
            } else {
                printf("Bボタン: 離された\n");
            }
            prev_button_b = button_b;
            state_changed = true;
        }

        // 状態が変化したらLCDを更新
        if (state_changed) {
            lcd.setCursor(0, 3);
            lcd.print("A:");
            lcd.print(!button_a ? "ON " : "OFF");
            lcd.print("  B:");
            lcd.print(!button_b ? "ON " : "OFF");
        }

        // チャタリング防止のため少し待機
        sleep_ms(10);
    }

    return 0;
}
