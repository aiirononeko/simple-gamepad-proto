/**
 * @file lcd_i2c.cpp
 * @brief I2C LCD 2004 (PCF8574) ドライバー実装
 */

#include "lcd_i2c.h"

LcdI2c::LcdI2c(i2c_inst_t* i2c, uint8_t addr)
    : _i2c(i2c), _addr(addr), _backlight(LCD_BACKLIGHT), _displayControl(0) {}

void LcdI2c::init() {
    sleep_ms(50);  // LCD電源安定待ち

    // 4ビットモードへの初期化シーケンス
    write4bits(0x03 << 4);
    sleep_ms(5);
    write4bits(0x03 << 4);
    sleep_ms(5);
    write4bits(0x03 << 4);
    sleep_us(150);
    write4bits(0x02 << 4);  // 4ビットモード設定

    // ファンクションセット: 4ビット、2ライン、5x8ドット
    sendCommand(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);

    // ディスプレイコントロール: ディスプレイON、カーソルOFF、ブリンクOFF
    _displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    sendCommand(LCD_DISPLAYCONTROL | _displayControl);

    // 画面クリア
    clear();

    // エントリモード: 左から右、シフトなし
    sendCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);

    home();
}

void LcdI2c::clear() {
    sendCommand(LCD_CLEARDISPLAY);
    sleep_ms(2);  // クリアコマンドは時間がかかる
}

void LcdI2c::home() {
    sendCommand(LCD_RETURNHOME);
    sleep_ms(2);
}

void LcdI2c::setCursor(uint8_t col, uint8_t row) {
    // 各行のDDRAMアドレスオフセット
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= LCD_ROWS) {
        row = LCD_ROWS - 1;
    }
    sendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LcdI2c::print(const char* str) {
    while (*str) {
        printChar(*str++);
    }
}

void LcdI2c::printChar(char c) {
    sendData(static_cast<uint8_t>(c));
}

void LcdI2c::backlightOn() {
    _backlight = LCD_BACKLIGHT;
    expanderWrite(0);
}

void LcdI2c::backlightOff() {
    _backlight = LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void LcdI2c::displayOn() {
    _displayControl |= LCD_DISPLAYON;
    sendCommand(LCD_DISPLAYCONTROL | _displayControl);
}

void LcdI2c::displayOff() {
    _displayControl &= ~LCD_DISPLAYON;
    sendCommand(LCD_DISPLAYCONTROL | _displayControl);
}

void LcdI2c::sendCommand(uint8_t cmd) {
    uint8_t high = cmd & 0xF0;
    uint8_t low = (cmd << 4) & 0xF0;
    write4bits(high);
    write4bits(low);
}

void LcdI2c::sendData(uint8_t data) {
    uint8_t high = (data & 0xF0) | LCD_RS;
    uint8_t low = ((data << 4) & 0xF0) | LCD_RS;
    write4bits(high);
    write4bits(low);
}

void LcdI2c::write4bits(uint8_t value) {
    expanderWrite(value);
    pulseEnable(value);
}

void LcdI2c::expanderWrite(uint8_t data) {
    uint8_t buf = data | _backlight;
    i2c_write_blocking(_i2c, _addr, &buf, 1, false);
}

void LcdI2c::pulseEnable(uint8_t data) {
    expanderWrite(data | LCD_EN);
    sleep_us(1);
    expanderWrite(data & ~LCD_EN);
    sleep_us(50);
}
