#include <lvgl.h>
#include "Arduino.h"
#include <time.h>
#include "utils.h"
#include "RTClib.h"

namespace ClockScreen
{
#if LV_FONT_MONTSERRAT_48
  static const lv_font_t *TIME_FONT = &lv_font_montserrat_48;
#else
  static const lv_font_t *TIME_FONT = &lv_font_montserrat_32;
#endif

  static lv_obj_t *timeLabel = nullptr;
  static lv_obj_t *dateLabel = nullptr;

  static lv_obj_t *batteryBorder = nullptr;
  static lv_obj_t *batteryFill = nullptr;
  static lv_obj_t *batteryTerminal = nullptr;

  static lv_timer_t *clockTimer = nullptr;
  static lv_timer_t *batteryTimer = nullptr;

  static void updateBatteryFill();

  static void makeStatic(lv_obj_t *obj)
  {
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
  }

  static void clockTimerCb(lv_timer_t *)
  {
    if (!timeLabel || !dateLabel)
      return;

    DateTime now = Utils::getDateTime();

    lv_label_set_text_fmt(
        timeLabel,
        "%02d:%02d",
        now.hour(),
        now.minute());

    static const char *DOW[] = {
        "SUN", "MON", "TUE", "WED",
        "THU", "FRI", "SAT"};

    static const char *MON[] = {
        "JAN", "FEB", "MAR", "APR",
        "MAY", "JUN", "JUL", "AUG",
        "SEP", "OCT", "NOV", "DEC"};

    lv_label_set_text_fmt(
        dateLabel,
        "%s  /  %02d %s",
        DOW[now.dayOfTheWeek()],
        now.day(),
        MON[now.month() - 1]);
  }

  static void batteryTimerCb(lv_timer_t *)
  {
    updateBatteryFill();
  }

  void createBatteryIcon(lv_obj_t *scr)
  {
    batteryBorder = lv_obj_create(scr);
    makeStatic(batteryBorder);

    lv_obj_set_size(batteryBorder, 38, 18);
    lv_obj_set_style_bg_opa(batteryBorder, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(batteryBorder, 2, 0);
    lv_obj_set_style_border_color(
        batteryBorder,
        lv_color_make(150, 160, 155),
        0);

    lv_obj_set_style_radius(batteryBorder, 4, 0);
    lv_obj_set_style_pad_all(batteryBorder, 0, 0);
    lv_obj_align(batteryBorder, LV_ALIGN_TOP_MID, 0, 30);

    batteryTerminal = lv_obj_create(scr);
    makeStatic(batteryTerminal);

    lv_obj_set_size(batteryTerminal, 3, 8);
    lv_obj_set_style_bg_color(
        batteryTerminal,
        lv_color_make(150, 160, 155),
        0);

    lv_obj_set_style_border_width(batteryTerminal, 0, 0);
    lv_obj_set_style_radius(batteryTerminal, 2, 0);

    lv_obj_align_to(
        batteryTerminal,
        batteryBorder,
        LV_ALIGN_OUT_RIGHT_MID,
        1,
        0);

    batteryFill = lv_obj_create(batteryBorder);
    makeStatic(batteryFill);

    lv_obj_set_pos(batteryFill, 2, 2);
    lv_obj_set_size(batteryFill, 34, 14);

    lv_obj_set_style_border_width(batteryFill, 0, 0);
    lv_obj_set_style_radius(batteryFill, 2, 0);
    lv_obj_set_style_pad_all(batteryFill, 0, 0);

    updateBatteryFill();
  }

  static void updateBatteryFill()
  {
    if (!batteryBorder || !batteryFill || !batteryTerminal)
      return;

    int pct = Utils::batteryPercent();

    if (pct < 0)
    {
      lv_obj_add_flag(batteryBorder, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(batteryTerminal, LV_OBJ_FLAG_HIDDEN);
      return;
    }

    lv_obj_remove_flag(batteryBorder, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(batteryTerminal, LV_OBJ_FLAG_HIDDEN);

    int fillW = (pct * 34) / 100;
    fillW = LV_CLAMP(0, fillW, 34);

    lv_obj_set_width(batteryFill, fillW);

    lv_color_t color;

    if (pct >= 60)
      color = lv_color_make(100, 220, 150);
    else if (pct >= 20)
      color = lv_color_make(245, 190, 70);
    else
      color = lv_color_make(245, 80, 80);

    lv_obj_set_style_bg_color(batteryFill, color, 0);
  }

  lv_obj_t *create()
  {
    lv_obj_t *scr = lv_obj_create(nullptr);

    makeStatic(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x151B19), 0);
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x080A09), 0);
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    timeLabel = lv_label_create(scr);
    lv_label_set_text(timeLabel, "--:--");

    lv_obj_set_style_text_color(
        timeLabel,
        lv_color_make(240, 245, 242),
        0);

    lv_obj_set_style_text_font(timeLabel, TIME_FONT, 0);
    lv_obj_set_style_text_letter_space(timeLabel, 1, 0);

    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -28);

    lv_obj_t *accentLine = lv_obj_create(scr);
    makeStatic(accentLine);

    lv_obj_set_size(accentLine, 76, 2);
    lv_obj_set_style_bg_color(
        accentLine,
        lv_color_make(105, 125, 115),
        0);

    lv_obj_set_style_border_width(accentLine, 0, 0);
    lv_obj_set_style_radius(accentLine, 2, 0);

    lv_obj_align(accentLine, LV_ALIGN_CENTER, 0, 21);

    lv_obj_t *dateCard = lv_obj_create(scr);
    makeStatic(dateCard);

    lv_obj_set_size(dateCard, 154, 32);
    lv_obj_set_style_bg_color(
        dateCard,
        lv_color_make(25, 30, 28),
        0);

    lv_obj_set_style_bg_opa(dateCard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dateCard, 1, 0);

    lv_obj_set_style_border_color(
        dateCard,
        lv_color_make(55, 65, 60),
        0);

    lv_obj_set_style_radius(dateCard, 16, 0);
    lv_obj_set_style_pad_all(dateCard, 0, 0);

    lv_obj_align(dateCard, LV_ALIGN_CENTER, 0, 55);

    dateLabel = lv_label_create(dateCard);
    lv_label_set_text(dateLabel, "---");

    lv_obj_set_style_text_color(
        dateLabel,
        lv_color_make(175, 185, 180),
        0);

    lv_obj_set_style_text_font(
        dateLabel,
        &lv_font_montserrat_18,
        0);

    lv_obj_center(dateLabel);

    createBatteryIcon(scr);

    clockTimer = lv_timer_create(clockTimerCb, 1000, nullptr);
    batteryTimer = lv_timer_create(batteryTimerCb, 10000, nullptr);

    clockTimerCb(nullptr);
    updateBatteryFill();

    return scr;
  }

  void update()
  {
  }

  void destroy(lv_obj_t *scr)
  {
    if (clockTimer)
    {
      lv_timer_delete(clockTimer);
      clockTimer = nullptr;
    }

    if (batteryTimer)
    {
      lv_timer_delete(batteryTimer);
      batteryTimer = nullptr;
    }

    timeLabel = nullptr;
    dateLabel = nullptr;
    batteryBorder = nullptr;
    batteryFill = nullptr;
    batteryTerminal = nullptr;

    lv_obj_delete(scr);
  }
}