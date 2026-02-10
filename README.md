# Simple Gamepad Proto

Raspberry Pi Pico を使用した **USB HID Gamepad** プロジェクト

## 特徴

- **汎用ゲームパッド**: 標準 HID Gamepad として認識される
- **2×2ボタンマトリクス×2**: ABXY + D-pad（計8ボタン）
- **最小構成**: 約120行のファームウェアコード

## ボタンマッピング

| 物理ボタン | Gamepad Index |
|-----------|--------------|
| A         | Button 1     |
| B         | Button 2     |
| X         | Button 3     |
| Y         | Button 4     |
| D-pad     | Hat Switch   |

## 必要な環境

- Raspberry Pi Pico SDK (pico-sdk)
- ARM GCC コンパイラ (arm-none-eabi-gcc)
- CMake (3.13以上)

## ビルド方法

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## 書き込み方法

1. Pico の **BOOTSEL** ボタンを押しながら USB ケーブルを接続
2. `build/simple-gamepad-proto.uf2` を RPI-RP2 ドライブにコピー
3. 自動的に再起動

## 動作確認

https://gamepadtester.net/ にアクセスして動作確認できます。

## プロジェクト構成

```
simple-gamepad-proto/
├── CMakeLists.txt
├── src/
│   ├── main.cpp            # メインプログラム（マトリクススキャン + HID送信）
│   ├── usb_descriptors.c   # USB HID Descriptor
│   └── tusb_config.h       # TinyUSB設定
└── schematics/             # 回路図
```
