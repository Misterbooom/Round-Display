#include <lvgl.h>
#include "icons/clear.h"
namespace WeatherScreen
{
  static lv_obj_t *temperatureLabel;
  static lv_obj_t *conditionLabel;
  static lv_obj_t *iconLabel;
  static lv_obj_t *detailsLabel;
  static lv_obj_t *weatherIcon;

  lv_obj_t *create()
  {
    lv_obj_t *scr = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 18, 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "WEATHER");
    lv_obj_set_style_text_color(title, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
    lv_obj_set_style_text_letter_space(title, 2, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    temperatureLabel = lv_label_create(scr);
    lv_label_set_text(temperatureLabel, "22°");
    lv_obj_set_style_text_color(temperatureLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(temperatureLabel, &lv_font_montserrat_32, 0);
    lv_obj_align(temperatureLabel, LV_ALIGN_CENTER, 0, -68);

    weatherIcon = lv_image_create(scr);
    lv_image_set_src(weatherIcon, &clear);
    lv_obj_align(weatherIcon, LV_ALIGN_CENTER, 0, 0);

    conditionLabel = lv_label_create(scr);
    lv_label_set_text(conditionLabel, "Sunny");
    lv_obj_set_style_text_color(conditionLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(conditionLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(conditionLabel, LV_ALIGN_CENTER, 0, 68);

    // Нижняя строка
    detailsLabel = lv_label_create(scr);
    lv_label_set_text(detailsLabel, "H: 25°   •   L: 16°");
    lv_obj_set_style_text_color(detailsLabel, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_obj_align(detailsLabel, LV_ALIGN_BOTTOM_MID, 0, -18);

    return scr;
  }

  void updateWeather(const char *temperature,
                     const char *condition,
                     const char *icon,
                     int high,
                     int low)
  {
    if (!temperatureLabel)
      return;

    lv_label_set_text(temperatureLabel, temperature);
    lv_label_set_text(conditionLabel, condition);
    lv_label_set_text(iconLabel, icon);

    lv_label_set_text_fmt(detailsLabel, "H: %d°   •   L: %d°", high, low);
  }

  void update()
  {
  }

  void destroy(lv_obj_t *scr)
  {
    temperatureLabel = nullptr;
    conditionLabel = nullptr;
    iconLabel = nullptr;
    detailsLabel = nullptr;
    weatherIcon = nullptr;

    lv_obj_delete(scr);
  }
}