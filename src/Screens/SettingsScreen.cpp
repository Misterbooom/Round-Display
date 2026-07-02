

#include <lvgl.h>
#include "ScreenManager.h"
#include "SettingsRenderer.h"

namespace SettingsScreen
{

  static void setMenuLayout(lv_obj_t *menu)
  {
    lv_obj_set_layout(menu, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);

    lv_obj_set_style_pad_top(menu, 2, 0);
    lv_obj_set_style_pad_bottom(menu, 2, 0);
    lv_obj_set_style_pad_row(menu, 8, 0);

    lv_obj_set_flex_align(
        menu,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
  }

  lv_obj_t *create()
  {
    lv_obj_t *scr = lv_obj_create(nullptr);
    makeStatic(scr);

    lv_obj_set_style_bg_opa(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t *menu = lv_obj_create(scr);
    setMenuLayout(menu);

    lv_obj_set_size(menu, 196, 166);
    lv_obj_set_style_bg_opa(menu, 0, 0);
    lv_obj_set_style_border_width(menu, 0, 0);
    lv_obj_set_style_pad_left(menu, 4, 0);
    lv_obj_set_style_pad_right(menu, 4, 0);

    lv_obj_set_scroll_dir(menu, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_OFF);

    lv_obj_align(menu, LV_ALIGN_TOP_MID, 0, 58);
    createTitle(scr, "SETTINGS");
    static const SettingsRenderer::MenuObj displayItems[] = {
        {SettingsRenderer::ObjType::Tab, "Clock", nullptr, 0},
        {SettingsRenderer::ObjType::Tab, "Weather", nullptr, 0},
        {SettingsRenderer::ObjType::Tab, "Timeout", nullptr, 0}};

    static const SettingsRenderer::MenuObj startPage = {
        SettingsRenderer::ObjType::Page,
        nullptr,
        displayItems,
        sizeof(displayItems) / sizeof(displayItems[0])};
    SettingsRenderer::renderObj(menu, startPage);
    return scr;
  }

  void update() { /* static */ }

  void destroy(lv_obj_t *scr)
  {
    lv_obj_delete(scr);
  }
}