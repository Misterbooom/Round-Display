/*
 * Settings Screen — one button that goes back to clock.
 * Add sliders, switches, etc. here later.
 */

#include <lvgl.h>
#include "ScreenManager.h"



namespace SettingsScreen
{

  lv_obj_t *create()
  {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_make(14, 15, 14), 0);



    return scr;
  }

  void update() { /* static */ }

  void destroy(lv_obj_t *scr) { lv_obj_delete(scr); }

}
