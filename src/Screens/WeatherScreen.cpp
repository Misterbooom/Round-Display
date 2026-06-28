/*
 * Weather Screen — placeholder. Shows a static label.
 * Replace with real weather data from BLE/WiFi when ready.
 */

#include <lvgl.h>

namespace WeatherScreen {

lv_obj_t *create()
{
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "Weather\n\n22 C\nSunny");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label);

  return scr;
}

void update() { /* static — nothing to update */ }

void destroy(lv_obj_t *scr) { lv_obj_delete(scr); }

}
