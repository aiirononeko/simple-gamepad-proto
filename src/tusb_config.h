/**
 * @file tusb_config.h
 * @brief TinyUSB設定ファイル - Gamepad用
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// 共通設定
//--------------------------------------------------------------------

// RP2040用
#define CFG_TUSB_MCU OPT_MCU_RP2040

// デバイスモード
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE

// OSなし（ベアメタル）
#define CFG_TUSB_OS OPT_OS_NONE

// デバッグ出力レベル（0=無効、1=エラー、2=警告、3=情報）
#define CFG_TUSB_DEBUG 0

// メモリアライメント
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

//--------------------------------------------------------------------
// デバイスクラス設定
//--------------------------------------------------------------------

// デバイスエンドポイント数
#define CFG_TUD_ENDPOINT0_SIZE 64

// 使用するデバイスクラス
#define CFG_TUD_HID 1
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

//--------------------------------------------------------------------
// HID クラス設定
//--------------------------------------------------------------------

// HID エンドポイントのバッファサイズ
#define CFG_TUD_HID_EP_BUFSIZE 16

#ifdef __cplusplus
}
#endif

#endif // _TUSB_CONFIG_H_
