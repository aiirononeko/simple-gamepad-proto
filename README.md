# Simple Gamepad Proto

Raspberry Pi Pico を使用した **DualShock 4互換 USB HID Gamepad** プロジェクト

## 特徴

- **DS4互換モード**: Sony DualShock 4として認識される（VID: 0x054C, PID: 0x05C4）
- **Gamepadtester対応**: PlayStationコントローラーUIが表示される
- **2x2ボタンマトリクス×2**: ABXY + D-pad

## ボタンマッピング

| 物理ボタン | DS4表示 | GamepadtesterのIndex |
|-----------|--------|---------------------|
| A         | ✕      | B1                  |
| B         | ○      | B2                  |
| X         | □      | B0                  |
| Y         | △      | B3                  |
| D-pad     | Hat    | 方向キー表示        |

## 必要な環境

- Raspberry Pi Pico SDK (pico-sdk)
- ARM GCC コンパイラ (arm-none-eabi-gcc)
- CMake (3.13以上)

## ビルド方法

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## 書き込み方法

1. Pico の **BOOTSEL** ボタンを押しながら USB ケーブルを接続
2. `build/simple-gamepad-proto.uf2` を RPI-RP2 ドライブにコピー
3. 自動的に再起動

## 動作確認

https://gamepadtester.net/ にアクセスして、PlayStation コントローラーUIで動作確認できます。

## プロジェクト構成

```
simple-gamepad-proto/
├── CMakeLists.txt
├── src/
│   ├── main.cpp            # メインプログラム（DS4互換）
│   ├── usb_descriptors.c   # USB HID Descriptor
│   ├── tusb_config.h       # TinyUSB設定
│   ├── lcd_i2c.cpp/h       # LCD制御
└── schematics/             # 回路図
```
