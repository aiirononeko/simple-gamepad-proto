---
description: 回路図の変更時にファイルを修正しビルドを実行する
---

# 回路図変更時のワークフロー

回路図に変更があった場合は、以下の手順に従ってください。

## 1. 回路図の確認と把握

まず、現在の回路図（`schematics/README.md`）を確認し、変更内容を把握します。

## 2. 回路図ファイルの修正

`schematics/README.md` を以下の観点で更新してください：

1. **部品リスト**: 新しい部品を追加、または既存部品を修正
2. **ピン配置**: ASCII アートのピン配置図を更新
3. **配線図**: 個別部品の配線図を更新
4. **完全な配線図**: 全体の配線図を更新
5. **注意事項**: 必要に応じて追加・修正

## 3. ソースコードとの整合性確認

回路図に対応するソースコードの変更が必要な場合は、以下のファイルを確認・修正してください：

- `src/main.cpp` - メインプログラム（GPIOピン設定など）
- `src/lcd_i2c.cpp` / `src/lcd_i2c.h` - I2C LCD 関連

## 4. ビルドの実行

// turbo
```bash
cd /home/ryota/ghq/github.com/aiirononeko/simple-gamepad-proto/build && cmake .. && make
```

ビルドが成功すると、`build/simple-gamepad-proto.uf2` ファイルが生成されます。

## 5. ビルド結果の確認

ビルドが成功したことを確認してください。エラーが発生した場合は、ソースコードを修正してから再度ビルドを実行してください。

## 関連ファイル

- 回路図: `schematics/README.md`
- メインプログラム: `src/main.cpp`
- LCD ライブラリ: `src/lcd_i2c.cpp`, `src/lcd_i2c.h`
- ビルド設定: `CMakeLists.txt`
- 出力ファイル: `build/simple-gamepad-proto.uf2`
