/**
 * @file lv_conf.h
 * LVGL v9 configuration for XIAO ESP32-S3 + Seeed Round Display (GC9A01 240×240)
 *
 * LV_CONF_INCLUDE_SIMPLE is set in platformio.ini build_flags.
 * That tells LVGL to do `#include "lv_conf.h"` — it finds this file
 * because include/ is in the build_flags include path (-I include).
 * Must live in include/ (not src/) so LVGL library compilation units find it.
 *
 * Generated from lv_conf_template.h shipped with LVGL v9.5.0
 */

/* clang-format off */
#if 1 /* ← MUST be 1! Template defaults to 0 which disables everything. */

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/

/** Color depth: 16 = RGB565 (GC9A01 uses 16-bit color over SPI) */
#define LV_COLOR_DEPTH 16

/** Swap byte order? 0 = no, GC9A01 is RGB565 native (MSB first) */
#define LV_COLOR_16_SWAP 0

/*=========================
   STDLIB — use LVGL's built-in memory manager
 *=========================*/
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

#define LV_STDINT_INCLUDE       <stdint.h>
#define LV_STDDEF_INCLUDE       <stddef.h>
#define LV_STDBOOL_INCLUDE      <stdbool.h>
#define LV_INTTYPES_INCLUDE     <inttypes.h>
#define LV_LIMITS_INCLUDE       <limits.h>
#define LV_STDARG_INCLUDE       <stdarg.h>

/** Heap for LVGL. ESP32-S3 has ~8MB PSRAM, so 128KB is very safe.
 *  Increase if you use many large images. */
#define LV_MEM_SIZE (128 * 1024U)

/*====================
   HAL SETTINGS
 *====================*/

/** Display refresh period. 33ms ≈ 30 FPS — smooth enough, saves CPU. */
#define LV_DEF_REFR_PERIOD  33

/** Dots per inch. Affects default widget sizes. 200 is reasonable for a
 *  240px round display that's physically ~1.28" diameter. */
#define LV_DPI_DEF 200

/*=================
 * OPERATING SYSTEM
 *=================*/
/** We're bare-metal Arduino — no RTOS. LVGL runs inside loop(). */
#define LV_USE_OS   LV_OS_NONE

/*========================
 * RENDERING CONFIGURATION
 *========================*/
#define LV_DRAW_BUF_STRIDE_ALIGN                1
#define LV_DRAW_BUF_ALIGN                       4
#define LV_DRAW_TRANSFORM_USE_MATRIX            0
#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE    (24 * 1024)
#define LV_DRAW_LAYER_MAX_MEMORY 0
#define LV_DRAW_THREAD_STACK_SIZE    (8 * 1024)
#define LV_DRAW_THREAD_PRIO LV_THREAD_PRIO_HIGH

/** Use software rendering (no GPU on ESP32-S3 for display). */
#define LV_USE_DRAW_SW 1
#if LV_USE_DRAW_SW == 1
    /** Only enable the color format we actually use (RGB565). Saves flash. */
    #define LV_DRAW_SW_SUPPORT_RGB565       1
    #define LV_DRAW_SW_SUPPORT_RGB565_SWAPPED 1
    #define LV_DRAW_SW_SUPPORT_RGB565A8     0
    #define LV_DRAW_SW_SUPPORT_RGB888       0
    #define LV_DRAW_SW_SUPPORT_XRGB8888     0
    #define LV_DRAW_SW_SUPPORT_ARGB8888     0
    #define LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED 0
    #define LV_DRAW_SW_SUPPORT_L8           0
    #define LV_DRAW_SW_SUPPORT_AL88         0
    #define LV_DRAW_SW_SUPPORT_A8           0
    #define LV_DRAW_SW_SUPPORT_I1           0

    #define LV_DRAW_SW_I1_LUM_THRESHOLD 127
    #define LV_DRAW_SW_DRAW_UNIT_CNT    1
    #define LV_USE_DRAW_ARM2D_SYNC      0
    #define LV_USE_NATIVE_HELIUM_ASM    0
    #define LV_DRAW_SW_COMPLEX          1
    #if LV_DRAW_SW_COMPLEX == 1
        #define LV_DRAW_SW_SHADOW_CACHE_SIZE 0
        #define LV_DRAW_SW_CIRCLE_CACHE_SIZE 4
    #endif
    #define LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NONE
    #define LV_USE_DRAW_SW_COMPLEX_GRADIENTS    0
#endif

/*========================
 * GPU — none on this board
 *========================*/
#define LV_USE_DRAW_OPENGLES    0
#define LV_USE_DRAW_VG_LITE     0
#define LV_USE_DRAW_VG_LITE_EXTERNAL 0
#define LV_USE_DRAW_SDL         0
#define LV_USE_DRAW_VITRO       0

/*==================
 * WIDGET SETTINGS
 *==================*/

/** Enable all commonly-used widgets. Disable what you don't need to save flash. */
#define LV_USE_ANIMIMG     1   /**< Animated image (not needed for basic UI) */
#define LV_USE_ARC         1   /**< Arc — great for round displays! */
#define LV_USE_BAR         1   /**< Progress bar */
#define LV_USE_BTN         1   /**< Button */
#define LV_USE_BTNMATRIX   0   /**< Button matrix / keypad */
#define LV_USE_CALENDAR    0   /**< Calendar widget */
#define LV_USE_CANVAS      0   /**< Canvas (free drawing) */
#define LV_USE_CHART       0   /**< Chart / graph */
#define LV_USE_CHECKBOX    0   /**< Checkbox */
#define LV_USE_DROPDOWN    0   /**< Drop-down list */
#define LV_USE_IMAGE       1   /**< Image (enable if you want to show bitmaps) */
#define LV_USE_IMAGEBUTTON 0   /**< Image button */
#define LV_USE_KEYBOARD    0   /**< On-screen keyboard */
#define LV_USE_LABEL       1   /**< Text label */
#define LV_USE_LED         0   /**< LED indicator */
#define LV_USE_LINE        1   /**< Line */
#define LV_USE_LIST        0   /**< List */
#define LV_USE_LOTTIE      0   /**< Lottie animations */
#define LV_USE_MENU        0   /**< Menu */
#define LV_USE_MSGBOX      0   /**< Message box / dialog */
#define LV_USE_ROLLER      0   /**< Roller / spinner */
#define LV_USE_SCALE       0   /**< Scale with tick marks */
#define LV_USE_SLIDER      1   /**< Slider */
#define LV_USE_SPAN        0   /**< Rich text spans */
#define LV_USE_SPINBOX     0   /**< Spinbox (number input with +/-) */
#define LV_USE_SPINNER     0   /**< Loading spinner */
#define LV_USE_SWITCH      1   /**< Toggle switch */
#define LV_USE_TEXTAREA    0   /**< Multi-line text input */
#define LV_USE_TABLE       0   /**< Table */
#define LV_USE_TABVIEW     0   /**< Tab view */
#define LV_USE_TILEVIEW    0   /**< Tile view */
#define LV_USE_WIN         0   /**< Window (title bar + close button) */

/*====================
 * EXTRA COMPONENTS
 *====================*/
#define LV_USE_FILE_EXPLORER 0
#define LV_USE_GRIDNAV       0
#define LV_USE_FRAGMENT      0
#define LV_USE_IME_PINYIN    0
#define LV_USE_OBSERVER      1   /**< Needed for event handling */
#define LV_USE_SNAPSHOT      0
#define LV_USE_TRANSLATIONS  0
#define LV_USE_MONKEY        0
#define LV_USE_GESTURE       0

/*==================
 * FONT USAGE
 *==================*/

/** Default font. Montserrat 14 is a good balance of readability and size.
 *  For a 240×240 round display with limited space, 14px is comfortable. */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/** Enable only the sizes you need. Each size costs ~10-50KB flash. */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1   /**< Default — must be enabled */
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1   /**< Nice for large titles */
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_48 1

/*==================
 * OTHER SETTINGS
 *==================*/
#define LV_USE_LOG 0           /**< Disable logging to save flash */
#define LV_USE_DEBUG 0
#define LV_USE_REFR_DEBUG 0
#define LV_USE_PERF_MONITOR 0

/** Layout engines — flex and grid are useful for responsive layouts. */
#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/*==================
 * EXTERNAL LIBRARIES — disable all (no file system, no PNG/JPG/SVG)
 *==================*/
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0
#define LV_USE_FS_LITTLEFS 0
#define LV_USE_FS_ARDUINO_ESP_LITTLEFS 0
#define LV_USE_FS_ARDUINO_SD 0

#define LV_USE_LODEPNG  0
#define LV_USE_LIBPNG   0
#define LV_USE_BMP      0
#define LV_USE_TJPGD    0
#define LV_USE_LIBJPEG_TURBO 0
#define LV_USE_GIF      0        /**< LVGL's built-in GIF widget — enable if you want lv_gif */
#define LV_USE_RLOTTIE  0
#define LV_USE_FFMPEG   0
#define LV_USE_FREETYPE 0
#define LV_USE_TINY_TTF 0

/*===================
 *  EXTERNAL LIBRARY PREFIX
 *===================*/
/** LVGL uses this prefix for any external library includes.
 *  Not strictly needed when all are disabled, but defines it to avoid compile errors. */
#define LV_ATTRIBUTE_LARGE_CONST

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
