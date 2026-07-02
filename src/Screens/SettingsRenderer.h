#pragma once
#include <lvgl.h>
namespace SettingsRenderer
{
  enum ObjType
  {

    Tab,
    Switch,
    Page,

  };
  struct MenuObj
  {
    ObjType objType;
    const char *title;

    const MenuObj *children;
    uint8_t childCount;
  };
  inline void createSettingsTab(lv_obj_t *menu, const char *tabTitle)
  {
    lv_obj_t *tab = lv_button_create(menu);

    lv_obj_set_size(tab, 184, 40);

    lv_obj_set_style_bg_color(tab, lv_color_hex(0x151B19), 0);
    lv_obj_set_style_bg_opa(tab, 220, 0);

    lv_obj_set_style_border_width(tab, 1, 0);
    lv_obj_set_style_border_color(tab, lv_color_hex(0x37433D), 0);
    lv_obj_set_style_border_opa(tab, 140, 0);

    lv_obj_set_style_radius(tab, 13, 0);
    lv_obj_set_style_pad_all(tab, 0, 0);

    lv_obj_set_style_shadow_width(tab, 7, 0);
    lv_obj_set_style_shadow_opa(tab, 50, 0);
    lv_obj_set_style_shadow_offset_y(tab, 2, 0);

    lv_obj_t *accent = lv_obj_create(tab);
    lv_obj_set_size(accent, 3, 16);
    lv_obj_set_style_radius(accent, 2, 0);
    lv_obj_set_style_bg_color(accent, lv_color_hex(0x718C7A), 0);
    lv_obj_set_style_bg_opa(accent, 180, 0);
    lv_obj_set_style_border_width(accent, 0, 0);
    lv_obj_align(accent, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t *tabLabel = lv_label_create(tab);
    lv_label_set_text(tabLabel, tabTitle);

    lv_obj_set_style_text_color(tabLabel, lv_color_hex(0xD1D8D3), 0);
    lv_obj_set_style_text_font(tabLabel, &lv_font_montserrat_18, 0);

    lv_obj_align(tabLabel, LV_ALIGN_LEFT_MID, 26, 0);

    lv_obj_t *detail = lv_obj_create(tab);
    lv_obj_set_size(detail, 16, 1);
    lv_obj_set_style_radius(detail, 1, 0);
    lv_obj_set_style_bg_color(detail, lv_color_hex(0x69766F), 0);
    lv_obj_set_style_bg_opa(detail, 100, 0);
    lv_obj_set_style_border_width(detail, 0, 0);
    lv_obj_align(detail, LV_ALIGN_RIGHT_MID, -14, 0);
  }
  inline void renderPage(lv_obj_t *menu, const MenuObj &pageObj)
  {
    for (int i = 0; i < pageObj.childCount; i++)
    {
      const MenuObj &child = pageObj.children[i];

      switch (child.objType)
      {
      case ObjType::Tab:
        createSettingsTab(menu, child.title);
        break;

      default:
        break;
      }
    }
  }
  inline void renderObj(lv_obj_t *menu, MenuObj obj)
  {
    switch (obj.objType)
    {
    case ObjType::Page:
      renderPage(menu, obj);
      break;
    default:
      break;
    }
  }
}