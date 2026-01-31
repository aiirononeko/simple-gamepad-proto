/**
 * @file lcd_i2c.h
 * @brief I2C LCD 2004 (PCF8574) ドライバー for Raspberry Pi Pico
 */

#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <cstdint>

// LCD 2004 設定
#define LCD_COLS 20
#define LCD_ROWS 4

// デフォルトI2Cアドレス (PCF8574: 0x27 または 0x3F)
#define LCD_DEFAULT_ADDR 0x27

// LCDコマンド
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// エントリモードフラグ
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// ディスプレイコントロールフラグ
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// ファンクションセットフラグ
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// バックライト
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

// Enable bit
#define LCD_EN 0x04
// Read/Write bit
#define LCD_RW 0x02
// Register select bit
#define LCD_RS 0x01

class LcdI2c {
public:
    /**
     * @brief コンストラクタ
     * @param i2c I2Cインスタンス (i2c0 または i2c1)
     * @param addr I2Cアドレス (デフォルト: 0x27)
     */
    LcdI2c(i2c_inst_t* i2c, uint8_t addr = LCD_DEFAULT_ADDR);

    /**
     * @brief LCD初期化
     */
    void init();

    /**
     * @brief 画面クリア
     */
    void clear();

    /**
     * @brief カーソルをホーム位置へ
     */
    void home();

    /**
     * @brief カーソル位置設定
     * @param col 列 (0-19)
     * @param row 行 (0-3)
     */
    void setCursor(uint8_t col, uint8_t row);

    /**
     * @brief 文字列を表示
     * @param str 表示する文字列
     */
    void print(const char* str);

    /**
     * @brief 1文字を表示
     * @param c 表示する文字
     */
    void printChar(char c);

    /**
     * @brief バックライト ON
     */
    void backlightOn();

    /**
     * @brief バックライト OFF
     */
    void backlightOff();

    /**
     * @brief ディスプレイ ON
     */
    void displayOn();

    /**
     * @brief ディスプレイ OFF
     */
    void displayOff();

private:
    i2c_inst_t* _i2c;
    uint8_t _addr;
    uint8_t _backlight;
    uint8_t _displayControl;

    void sendCommand(uint8_t cmd);
    void sendData(uint8_t data);
    void write4bits(uint8_t value);
    void expanderWrite(uint8_t data);
    void pulseEnable(uint8_t data);
};

#endif // LCD_I2C_H
