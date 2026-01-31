/**
 * @file main.cpp
 * @brief Raspberry Pi Pico USB HID Gamepad - DualShock 4互換
 *
 * DS4互換モードで動作するゲームパッド。
 * GamepadtesterでPlayStationコントローラーUIが表示されます。
 * 
 * ボタン配置:
 * 【D-pad】          【ABXYボタン → DS4マッピング】
 *    [↑]                    [Y] → △
 * [←]   [→]             [X] → □   [B] → ○
 *    [↓]                    [A] → ✕
 *
 * ハードウェア接続:
 * ABXYマトリクス:
 *           COL0(GP16)  COL1(GP17)
 * ROW0(GP14)    Y(△)       B(○)
 * ROW1(GP15)    X(□)       A(✕)
 *
 * D-padマトリクス:
 *               COL0(GP20)  COL1(GP21)
 * DPAD_ROW0(GP18)   ↑           →
 * DPAD_ROW1(GP19)   ↓           ←
 * 
 * DS4ボタンマッピング:
 * | Button | Index | 物理ボタン |
 * |--------|-------|-----------|
 * | □      | B0    | X         |
 * | ✕      | B1    | A         |
 * | ○      | B2    | B         |
 * | △      | B3    | Y         |
 * | D-pad  | Hat   | 十字キー   |
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_i2c.h"
#include "tusb.h"
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
 * DS4互換 HID レポート構造体
 * 
 * Report構造 (6 bytes total):
 * - report_id: 1
 * - buttons: 14ボタン + 2bit padding
 * - hat: D-pad (4 bits) + 4bit padding
 * - x, y: 左スティック (0-255, 中央=128)
 */
typedef struct __attribute__((packed)) {
    uint8_t report_id;  // Report ID (always 1)
    uint16_t buttons;   // 14 buttons + 2 reserved bits
    uint8_t hat;        // Hat switch (4 bits) + padding (4 bits)
    uint8_t x;          // X axis (0-255, center=128)
    uint8_t y;          // Y axis (0-255, center=128)
} ds4_report_t;

static ds4_report_t gamepad_report = {1, 0, 0x08, 128, 128};  // Hat=8 is neutral
static ds4_report_t prev_gamepad_report = {1, 0, 0x08, 128, 128};

/**
 * D-padの状態からHat Switch値を計算
 * DS4 Hat Switch: 0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW, 8=neutral
 * @return Hat switch value (0-7: direction, 8: neutral)
 */
uint8_t get_hat_value() {
    bool up = dpad_state[DPAD_UP];
    bool right = dpad_state[DPAD_RIGHT];
    bool down = dpad_state[DPAD_DOWN];
    bool left = dpad_state[DPAD_LEFT];

    if (up && right) return 1;   // NE
    if (right && down) return 3; // SE
    if (down && left) return 5;  // SW
    if (left && up) return 7;    // NW
    if (up) return 0;            // N
    if (right) return 2;         // E
    if (down) return 4;          // S
    if (left) return 6;          // W
    return 8;                    // Neutral (null state)
}

/**
 * ボタン状態からDS4互換ボタンビットフィールドを生成
 * DS4配列: □(B0) ✕(B1) ○(B2) △(B3)
 * @return Button bit field (16 bits, upper 2 bits unused)
 */
uint16_t get_buttons_value() {
    uint16_t buttons = 0;
    // DS4マッピング: □=X, ✕=A, ○=B, △=Y
    if (button_state[BTN_X]) buttons |= (1 << 0);  // □ Square
    if (button_state[BTN_A]) buttons |= (1 << 1);  // ✕ Cross
    if (button_state[BTN_B]) buttons |= (1 << 2);  // ○ Circle
    if (button_state[BTN_Y]) buttons |= (1 << 3);  // △ Triangle
    // B4-B13: L1, R1, L2, R2, Share, Options, L3, R3, PS, Touchpad (未実装)
    return buttons;
}

/**
 * ABXYマトリクススキャン
 */
void scan_abxy_matrix() {
    gpio_put(ROW0_PIN, 0);
    gpio_put(ROW1_PIN, 1);
    sleep_us(10);
    button_state[BTN_Y] = !gpio_get(COL0_PIN);
    button_state[BTN_B] = !gpio_get(COL1_PIN);
    
    gpio_put(ROW0_PIN, 1);
    gpio_put(ROW1_PIN, 0);
    sleep_us(10);
    button_state[BTN_X] = !gpio_get(COL0_PIN);
    button_state[BTN_A] = !gpio_get(COL1_PIN);
    
    gpio_put(ROW0_PIN, 1);
    gpio_put(ROW1_PIN, 1);
}

/**
 * D-padマトリクススキャン
 */
void scan_dpad_matrix() {
    gpio_put(DPAD_ROW0_PIN, 0);
    gpio_put(DPAD_ROW1_PIN, 1);
    sleep_us(10);
    dpad_state[DPAD_UP] = !gpio_get(DPAD_COL0_PIN);
    dpad_state[DPAD_RIGHT] = !gpio_get(DPAD_COL1_PIN);
    
    gpio_put(DPAD_ROW0_PIN, 1);
    gpio_put(DPAD_ROW1_PIN, 0);
    sleep_us(10);
    dpad_state[DPAD_LEFT] = !gpio_get(DPAD_COL0_PIN);
    dpad_state[DPAD_DOWN] = !gpio_get(DPAD_COL1_PIN);
    
    gpio_put(DPAD_ROW0_PIN, 1);
    gpio_put(DPAD_ROW1_PIN, 1);
}

/**
 * HIDレポートを送信
 */
void send_hid_report() {
    gamepad_report.buttons = get_buttons_value();
    gamepad_report.hat = get_hat_value();
    gamepad_report.x = 128;  // Joystick未実装（中央）
    gamepad_report.y = 128;  // Joystick未実装（中央）

    // 変化がある場合のみ送信
    if (gamepad_report.buttons != prev_gamepad_report.buttons ||
        gamepad_report.hat != prev_gamepad_report.hat ||
        gamepad_report.x != prev_gamepad_report.x ||
        gamepad_report.y != prev_gamepad_report.y) {
        
        if (tud_hid_ready()) {
            tud_hid_report(0, &gamepad_report, sizeof(gamepad_report));
        }
        prev_gamepad_report = gamepad_report;
    }
}



int main() {
    stdio_init_all();
    
    // TinyUSB初期化
    tusb_init();


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
    lcd.print("USB HID Gamepad");
    lcd.setCursor(0, 1);
    lcd.print("ABXY + D-pad");
    lcd.setCursor(0, 2);
    lcd.print("ABXY:");
    lcd.setCursor(0, 3);
    lcd.print("DPAD:");

    printf("USB HID Gamepad 起動完了\n");

    // メインループ
    while (true) {
        // TinyUSBタスク処理
        tud_task();
        
        scan_abxy_matrix();
        scan_dpad_matrix();

        // HIDレポート送信
        send_hid_report();

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

        sleep_ms(1);  // ポーリングレートを上げる
    }

    return 0;
}
