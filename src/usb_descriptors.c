/**
 * @file usb_descriptors.c
 * @brief USB Descriptor定義 - DualShock 4互換 Gamepad
 * 
 * このファイルはUSB HIDデバイスとしてPCに認識されるための
 * ディスクリプタを定義します。
 * 
 * DS4互換モード:
 * - VID/PID: Sony DualShock 4 (0x054C / 0x05C4)
 * - ボタンマッピング: DS4標準配列
 * - GamepadtesterでPlayStationコントローラーUIが表示されます
 * 
 * ボタンマッピング（DS4標準）:
 * | Button | Index | 説明           |
 * |--------|-------|----------------|
 * | □      | B0    | Square         |
 * | ✕      | B1    | Cross          |
 * | ○      | B2    | Circle         |
 * | △      | B3    | Triangle       |
 * | L1     | B4    | Left Bumper    |
 * | R1     | B5    | Right Bumper   |
 * | L2     | B6    | Left Trigger   |
 * | R2     | B7    | Right Trigger  |
 * | Share  | B8    | Share Button   |
 * | Options| B9    | Options Button |
 * | L3     | B10   | Left Stick     |
 * | R3     | B11   | Right Stick    |
 * | PS     | B12   | PS Button      |
 * | Touchpad| B13  | Touchpad Click |
 * | D-pad  | Hat   | 8方向 + 中立   |
 * 
 * 現在の実装:
 * - ABXY → □✕○△ (B0-B3)
 * - D-pad → Hat Switch (8方向)
 * - X/Y軸 → 左スティック（将来のJoystick用）
 */

#include "tusb.h"
#include <string.h>

//--------------------------------------------------------------------
// デバイスDescriptor - DualShock 4互換
//--------------------------------------------------------------------

tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    // Sony DualShock 4 VID/PID
    .idVendor = 0x054C,   // Sony Corporation
    .idProduct = 0x05C4,  // DualShock 4 [CUH-ZCT1x]

    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device;
}

//--------------------------------------------------------------------
// HID Report Descriptor - DS4互換
//--------------------------------------------------------------------

/**
 * DS4互換 HID Report Descriptor
 * 
 * Report構造 (5 bytes):
 * - Byte 0: buttons_low (□✕○△ L1 R1 L2 R2)
 * - Byte 1: buttons_high (Share Options L3 R3 PS Touchpad + 2 reserved)
 * - Byte 2: hat (D-pad, 4 bits) + reserved (4 bits)
 * - Byte 3: X axis (左スティック)
 * - Byte 4: Y axis (左スティック)
 */
uint8_t const desc_hid_report[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)

    // ---- 14 Buttons ----
    // DS4配列: □✕○△ L1 R1 L2 R2 Share Options L3 R3 PS Touchpad
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x0E,        //   Usage Maximum (Button 14)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data, Var, Abs)

    // ---- Padding (2 bits) ----
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x03,        //   Input (Const, Var, Abs)

    // ---- Hat Switch (D-pad) ----
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x39,        //   Usage (Hat Switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315 degrees)
    0x65, 0x14,        //   Unit (Degrees)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data, Var, Abs, Null State)

    // ---- Padding (4 bits) ----
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const, Var, Abs)

    // ---- X/Y Axes (Left Stick) ----
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data, Var, Abs)

    0xC0               // End Collection
};

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return desc_hid_report;
}

//--------------------------------------------------------------------
// Configuration Descriptor
//--------------------------------------------------------------------

enum {
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

//--------------------------------------------------------------------
// String Descriptors
//--------------------------------------------------------------------

char const* string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: Language (English)
    "Sony Interactive Entertainment",  // 1: Manufacturer (DS4互換)
    "Wireless Controller",             // 2: Product (DS4互換)
    "000001",                          // 3: Serial
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
            return NULL;
        }
        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}

//--------------------------------------------------------------------
// HID Callbacks
//--------------------------------------------------------------------

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}


