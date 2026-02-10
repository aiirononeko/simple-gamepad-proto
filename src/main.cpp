/**
 * @file main.cpp
 * @brief Raspberry Pi Pico USB HID Gamepad（最小構成）
 *
 * 2×2ボタンマトリクス×2 で ABXY + D-pad を実装。
 * 汎用ゲームパッドとして認識される。
 *
 * マトリクス:
 *   ABXY:  ROW0(GP14)/ROW1(GP15) × COL0(GP16)/COL1(GP17)
 *   D-pad: ROW0(GP18)/ROW1(GP19) × COL0(GP20)/COL1(GP21)
 */

#include "pico/stdlib.h"
#include "tusb.h"

// --- ピン定義 ---
// ABXY
#define ROW0 14
#define ROW1 15
#define COL0 16
#define COL1 17
// D-pad
#define DROW0 18
#define DROW1 19
#define DCOL0 20
#define DCOL1 21

// --- HID レポート (4 bytes, Report ID なし) ---
typedef struct __attribute__((packed)) {
  uint8_t buttons; // bit0=A, bit1=B, bit2=X, bit3=Y
  uint8_t hat;     // 0-7=方向, 0xF=ニュートラル
  uint8_t x;       // 左スティックX (中央=128)
  uint8_t y;       // 左スティックY (中央=128)
} gamepad_report_t;

static gamepad_report_t report = {0, 0x0F, 128, 128};
static gamepad_report_t prev_report = {0, 0x0F, 128, 128};

// --- マトリクススキャン ---
// 2本のROWを順番にLOWにして、2本のCOLを読む → 4ボタン分
static void scan_matrix(uint row0, uint row1, uint col0, uint col1,
                        bool out[4]) {
  // ROW0 = LOW → COL0, COL1 を読む
  gpio_put(row0, 0);
  gpio_put(row1, 1);
  sleep_us(5);
  out[0] = !gpio_get(col0); // ROW0×COL0
  out[1] = !gpio_get(col1); // ROW0×COL1

  // ROW1 = LOW → COL0, COL1 を読む
  gpio_put(row0, 1);
  gpio_put(row1, 0);
  sleep_us(5);
  out[2] = !gpio_get(col0); // ROW1×COL0
  out[3] = !gpio_get(col1); // ROW1×COL1

  gpio_put(row0, 1);
  gpio_put(row1, 1);
}

// D-pad → Hat Switch 変換
static uint8_t dpad_to_hat(bool up, bool right, bool down, bool left) {
  if (up && right)
    return 1;
  if (right && down)
    return 3;
  if (down && left)
    return 5;
  if (left && up)
    return 7;
  if (up)
    return 0;
  if (right)
    return 2;
  if (down)
    return 4;
  if (left)
    return 6;
  return 0x0F; // ニュートラル (Null state)
}

// GPIO初期化ヘルパー
static void init_row(uint pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, 1);
}
static void init_col(uint pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_up(pin);
}

int main() {
  stdio_init_all();
  tusb_init();

  // ABXY
  init_row(ROW0);
  init_row(ROW1);
  init_col(COL0);
  init_col(COL1);
  // D-pad
  init_row(DROW0);
  init_row(DROW1);
  init_col(DCOL0);
  init_col(DCOL1);

  while (true) {
    tud_task();

    // ABXY: [0]=Y, [1]=B, [2]=X, [3]=A
    bool abxy[4];
    scan_matrix(ROW0, ROW1, COL0, COL1, abxy);

    // D-pad: [0]=↑, [1]=→, [2]=←, [3]=↓
    bool dpad[4];
    scan_matrix(DROW0, DROW1, DCOL0, DCOL1, dpad);

    // レポート構築
    report.buttons = 0;
    if (abxy[3])
      report.buttons |= (1 << 0); // A
    if (abxy[1])
      report.buttons |= (1 << 1); // B
    if (abxy[2])
      report.buttons |= (1 << 2); // X
    if (abxy[0])
      report.buttons |= (1 << 3); // Y

    // D-pad: up=[0], right=[1], down=[3], left=[2]
    report.hat = dpad_to_hat(dpad[0], dpad[1], dpad[3], dpad[2]);

    report.x = 128;
    report.y = 128;

    // 変化時のみ送信
    if (report.buttons != prev_report.buttons ||
        report.hat != prev_report.hat) {
      if (tud_hid_ready()) {
        tud_hid_report(0, &report, sizeof(report));
      }
      prev_report = report;
    }

    sleep_ms(1);
  }
  return 0;
}
