#pragma once
#include <lvgl.h>

extern "C"
{
  extern const lv_image_dsc_t clear;
  extern const lv_image_dsc_t heavy_rain;
  extern const lv_image_dsc_t heavy_showers;
  extern const lv_image_dsc_t heavy_snow;
  extern const lv_image_dsc_t mostly_clear;
  extern const lv_image_dsc_t mostly_clear;
  extern const lv_image_dsc_t partly_cloudy;
  extern const lv_image_dsc_t rain;
  extern const lv_image_dsc_t showers;
  extern const lv_image_dsc_t snow;
}

namespace Icons
{
  inline const lv_image_dsc_t &clear = ::clear;
  inline const lv_image_dsc_t &heavy_rain = ::heavy_rain;
  inline const lv_image_dsc_t &heavy_showers = ::heavy_showers;
  inline const lv_image_dsc_t &heavy_snow = ::heavy_snow;
  inline const lv_image_dsc_t &mostly_clear = ::mostly_clear;
  inline const lv_image_dsc_t &partly_cloudy = ::partly_cloudy;
  inline const lv_image_dsc_t &rain = ::rain;
  inline const lv_image_dsc_t &showers = ::showers;
  inline const lv_image_dsc_t &snow = ::snow;
}