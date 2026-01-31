/**
 * @file main.cpp
 * @brief Raspberry Pi Pico ゲームパッド - 2x2ボタンマトリクス×2
 *
 * ボタン配置:
 * 【D-pad】          【ABXYボタン（Xbox配置）】
 *    [↑]                    [Y]
 * [←]   [→]             [X]   [B]
 *    [↓]                    [A]
 *
 * ABXYマトリクス:
 *           COL0(GP16)  COL1(GP17)
 * ROW0(GP14)    Y           B
 * ROW1(GP15)    X           A
 *
 * D-padマトリクス:
 *               COL0(GP20)  COL1(GP21)
 * DPAD_ROW0(GP18)   ↑           →
 * DPAD_ROW1(GP19)   ↓           ←
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_i2c.h"
#include <cstdio>

// ABXYマトリクスピン
#define ROW0_PIN 14  // GP14: Y, B
#define ROW1_PIN 15  // GP15: X, A
#define COL0_PIN 16  // GP16: Y, X
#define COL1_PIN 17  // GP17: B, A

// D-padマトリクスピン
#define DPAD_ROW0_PIN 18  // GP18: 上, 右
#define DPAD_ROW1_PIN 19  // GP19: 左, 下
#define DPAD_COL0_PIN 20  // GP20: 上, 左
#define DPAD_COL1_PIN 21  // GP21: 右, 下

// I2Cピン
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_BAUDRATE 100000

// LCD I2Cアドレス
#define LCD_ADDR 0x27

// ボタンインデックス（ABXY）
#define BTN_Y 0
#define BTN_B 1
#define BTN_X 2
#define BTN_A 3

// ボタンインデックス（D-pad）
#define DPAD_UP    0
#define DPAD_RIGHT 1
#define DPAD_LEFT  2
#define DPAD_DOWN  3

// ボタン状態
bool button_state[4] = {false, false, false, false};
bool prev_button_state[4] = {false, false, false, false};
const char* button_names[4] = {"Y", "B", "X", "A"};

// D-pad状態
bool dpad_state[4] = {false, false, false, false};
bool prev_dpad_state[4] = {false, false, false, false};
const char* dpad_names[4] = {"UP", "RIGHT", "LEFT", "DOWN"};

/**
 * ABXYマトリクススキャン
 */
void scan_abxy_matrix() {
    // ROW0スキャン (Y, B)
    gpio_put(ROW0_PIN, 0);
    gpio_put(ROW1_PIN, 1);
    sleep_us(10);
    button_state[BTN_Y] = !gpio_get(COL0_PIN);
    button_state[BTN_B] = !gpio_get(COL1_PIN);
    
    // ROW1スキャン (X, A)
    gpio_put(ROW0_PIN, 1);
    gpio_put(ROW1_PIN, 0);
    sleep_us(10);
    button_state[BTN_X] = !gpio_get(COL0_PIN);
    button_state[BTN_A] = !gpio_get(COL1_PIN);
    
    // スキャン終了
    gpio_put(ROW0_PIN, 1);
    gpio_put(ROW1_PIN, 1);
}

/**
 * D-padマトリクススキャン
 */
void scan_dpad_matrix() {
    // DPAD_ROW0スキャン (上, 右)
    gpio_put(DPAD_ROW0_PIN, 0);
    gpio_put(DPAD_ROW1_PIN, 1);
    sleep_us(10);
    dpad_state[DPAD_UP] = !gpio_get(DPAD_COL0_PIN);
    dpad_state[DPAD_RIGHT] = !gpio_get(DPAD_COL1_PIN);
    
    // DPAD_ROW1スキャン (左, 下)
    gpio_put(DPAD_ROW0_PIN, 1);
    gpio_put(DPAD_ROW1_PIN, 0);
    sleep_us(10);
    dpad_state[DPAD_LEFT] = !gpio_get(DPAD_COL0_PIN);
    dpad_state[DPAD_DOWN] = !gpio_get(DPAD_COL1_PIN);
    
    // スキャン終了
    gpio_put(DPAD_ROW0_PIN, 1);
    gpio_put(DPAD_ROW1_PIN, 1);
}

int main() {
    stdio_init_all();

    // ABXY ROWピン初期化（出力、初期HIGH）
    gpio_init(ROW0_PIN);
    gpio_set_dir(ROW0_PIN, GPIO_OUT);
    gpio_put(ROW0_PIN, 1);
    
    gpio_init(ROW1_PIN);
    gpio_set_dir(ROW1_PIN, GPIO_OUT);
    gpio_put(ROW1_PIN, 1);

    // ABXY COLピン初期化（入力、プルアップ）
    gpio_init(COL0_PIN);
    gpio_set_dir(COL0_PIN, GPIO_IN);
    gpio_pull_up(COL0_PIN);
    
    gpio_init(COL1_PIN);
    gpio_set_dir(COL1_PIN, GPIO_IN);
    gpio_pull_up(COL1_PIN);

    // D-pad ROWピン初期化（出力、初期HIGH）
    gpio_init(DPAD_ROW0_PIN);
    gpio_set_dir(DPAD_ROW0_PIN, GPIO_OUT);
    gpio_put(DPAD_ROW0_PIN, 1);
    
    gpio_init(DPAD_ROW1_PIN);
    gpio_set_dir(DPAD_ROW1_PIN, GPIO_OUT);
    gpio_put(DPAD_ROW1_PIN, 1);

    // D-pad COLピン初期化（入力、プルアップ）
    gpio_init(DPAD_COL0_PIN);
    gpio_set_dir(DPAD_COL0_PIN, GPIO_IN);
    gpio_pull_up(DPAD_COL0_PIN);
    
    gpio_init(DPAD_COL1_PIN);
    gpio_set_dir(DPAD_COL1_PIN, GPIO_IN);
    gpio_pull_up(DPAD_COL1_PIN);

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

    // 起動メッセージ
    lcd.setCursor(0, 0);
    lcd.print("Simple Gamepad");
    lcd.setCursor(0, 1);
    lcd.print("ABXY + D-pad");
    lcd.setCursor(0, 2);
    lcd.print("ABXY:");
    lcd.setCursor(0, 3);
    lcd.print("DPAD:");

    printf("Simple Gamepad 起動完了\n");

    // メインループ
    while (true) {
        scan_abxy_matrix();
        scan_dpad_matrix();

        bool state_changed = false;

        // ABXY状態チェック
        for (int i = 0; i < 4; i++) {
            if (button_state[i] != prev_button_state[i]) {
                printf("%sボタン: %s\n", button_names[i], 
                       button_state[i] ? "押された" : "離された");
                prev_button_state[i] = button_state[i];
                state_changed = true;
            }
        }

        // D-pad状態チェック
        for (int i = 0; i < 4; i++) {
            if (dpad_state[i] != prev_dpad_state[i]) {
                printf("D-pad %s: %s\n", dpad_names[i], 
                       dpad_state[i] ? "押された" : "離された");
                prev_dpad_state[i] = dpad_state[i];
                state_changed = true;
            }
        }

        // LCD更新
        if (state_changed) {
            // ABXY行
            lcd.setCursor(5, 2);
            lcd.print("               ");
            lcd.setCursor(5, 2);
            bool any_abxy = false;
            for (int i = 0; i < 4; i++) {
                if (button_state[i]) {
                    lcd.print(button_names[i]);
                    lcd.print(" ");
                    any_abxy = true;
                }
            }
            if (!any_abxy) lcd.print("-");

            // D-pad行
            lcd.setCursor(5, 3);
            lcd.print("               ");
            lcd.setCursor(5, 3);
            bool any_dpad = false;
            for (int i = 0; i < 4; i++) {
                if (dpad_state[i]) {
                    lcd.print(dpad_names[i]);
                    lcd.print(" ");
                    any_dpad = true;
                }
            }
            if (!any_dpad) lcd.print("-");
        }

        sleep_ms(10);
    }

    return 0;
}

