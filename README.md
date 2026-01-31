# Simple Gamepad Proto

Raspberry Pi Pico を使用した C++ 開発プロジェクト

## 必要な環境

- **Raspberry Pi Pico SDK** (pico-sdk)
- **ARM GCC コンパイラ** (arm-none-eabi-gcc)
- **CMake** (3.13以上)
- **Make** または **Ninja**

## セットアップ

### 1. Pico SDK のインストール

```bash
# SDK をクローン
cd ~
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

# 環境変数を設定（.bashrc や .zshrc に追加）
export PICO_SDK_PATH=~/pico-sdk
```

### 2. ツールチェーンのインストール

```bash
# Ubuntu/Debian の場合
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
```

## ビルド方法

```bash
# ビルドディレクトリを作成
mkdir build
cd build

# CMake を実行
cmake ..

# ビルド
make -j$(nproc)
```

## 書き込み方法

1. Pico の **BOOTSEL** ボタンを押しながら USB ケーブルを接続
2. Pico がマスストレージデバイスとして認識される
3. `build/simple-gamepad-proto.uf2` を Pico ドライブにコピー
4. 自動的に再起動して実行開始

```bash
# Linux の場合（Pico が /media/<user>/RPI-RP2 にマウントされている場合）
cp build/simple-gamepad-proto.uf2 /media/$USER/RPI-RP2/
```

## プロジェクト構成

```
simple-gamepad-proto/
├── CMakeLists.txt          # メインの CMake 設定
├── pico_sdk_import.cmake   # Pico SDK インポートスクリプト
├── src/
│   └── main.cpp            # メインプログラム
└── README.md               # このファイル
```

## デバッグ（シリアル出力）

USB シリアルを使用してデバッグ出力を確認できます：

```bash
# minicom を使用する場合
sudo minicom -b 115200 -D /dev/ttyACM0

# screen を使用する場合
sudo screen /dev/ttyACM0 115200
```

## ライセンス

MIT License
