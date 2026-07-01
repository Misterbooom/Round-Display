#include "ScreenManager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>
#include <stdio.h>

namespace WeatherScreen
{
#if LV_FONT_MONTSERRAT_48
  static const lv_font_t *TEMP_FONT = &lv_font_montserrat_48;
#else
  static const lv_font_t *TEMP_FONT = &lv_font_montserrat_32;
#endif

#if LV_FONT_MONTSERRAT_14
  static const lv_font_t *VALUE_FONT = &lv_font_montserrat_14;
#else
  static const lv_font_t *VALUE_FONT = &lv_font_montserrat_12;
#endif

  static constexpr int CARD_WIDTH = 88;
  static constexpr int CARD_HEIGHT = 60;

  static lv_obj_t *temperatureLabel = nullptr;
  static lv_obj_t *humidityValueLabel = nullptr;
  static lv_obj_t *windValueLabel = nullptr;

  struct WeatherData
  {
    float temperatureC = NAN;
    int humidityPercent = -1;
    float windKmh = NAN;
  };

  static WeatherData cachedWeather;

  static bool weatherAvailable = false;
  static bool weatherDirty = false;

  static portMUX_TYPE weatherMux = portMUX_INITIALIZER_UNLOCKED;

  static void makeStatic(lv_obj_t *obj)
  {
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
  }

  static lv_obj_t *createMetricCard(
      lv_obj_t *scr,
      int xOffset,
      const char *title,
      const char *unit)
  {
    lv_obj_t *card = lv_obj_create(scr);
    makeStatic(card);

    lv_obj_set_size(card, CARD_WIDTH, CARD_HEIGHT);

    lv_obj_set_style_bg_color(
        card,
        lv_color_make(25, 30, 28),
        0);

    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);

    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(
        card,
        lv_color_make(55, 65, 60),
        0);

    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_all(card, 0, 0);

    lv_obj_align(card, LV_ALIGN_CENTER, xOffset, 55);

    lv_obj_t *titleLabel = lv_label_create(card);
    lv_label_set_text(titleLabel, title);

    lv_obj_set_width(titleLabel, CARD_WIDTH - 8);
    lv_obj_set_style_text_align(titleLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(
        titleLabel,
        lv_color_make(135, 155, 145),
        0);

    lv_obj_set_style_text_font(
        titleLabel,
        &lv_font_montserrat_10,
        0);

    lv_obj_set_style_text_letter_space(titleLabel, 1, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 6);

    lv_obj_t *valueLabel = lv_label_create(card);
    lv_label_set_text(valueLabel, "--");

    lv_obj_set_width(valueLabel, CARD_WIDTH - 8);
    lv_obj_set_style_text_align(valueLabel, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_set_style_text_color(
        valueLabel,
        lv_color_make(240, 245, 242),
        0);

    lv_obj_set_style_text_font(valueLabel, VALUE_FONT, 0);
    lv_obj_align(valueLabel, LV_ALIGN_CENTER, 0, 4);

    lv_obj_t *unitLabel = lv_label_create(card);
    lv_label_set_text(unitLabel, unit);

    lv_obj_set_style_text_color(
        unitLabel,
        lv_color_make(105, 125, 115),
        0);

    lv_obj_set_style_text_font(
        unitLabel,
        &lv_font_montserrat_10,
        0);

    lv_obj_align(unitLabel, LV_ALIGN_BOTTOM_MID, 0, -5);

    return valueLabel;
  }

  static void renderWeather(const WeatherData &weather)
  {
    if (!temperatureLabel ||
        !humidityValueLabel ||
        !windValueLabel)
    {
      return;
    }

    if (!isfinite(weather.temperatureC))
    {
      lv_label_set_text(temperatureLabel, "--°");
    }
    else
    {
      int tempTenths = static_cast<int>(
          lroundf(weather.temperatureC * 10.0f));

      int absoluteTenths =
          tempTenths < 0 ? -tempTenths : tempTenths;

      char text[16];

      snprintf(
          text,
          sizeof(text),
          "%s%d.%d°",
          tempTenths < 0 ? "-" : "",
          absoluteTenths / 10,
          absoluteTenths % 10);

      lv_label_set_text(temperatureLabel, text);
    }

    if (weather.humidityPercent < 0)
    {
      lv_label_set_text(humidityValueLabel, "--");
    }
    else
    {
      lv_label_set_text_fmt(
          humidityValueLabel,
          "%d",
          weather.humidityPercent);
    }

    if (!isfinite(weather.windKmh))
    {
      lv_label_set_text(windValueLabel, "--");
    }
    else
    {
      lv_label_set_text_fmt(
          windValueLabel,
          "%d",
          static_cast<int>(lroundf(weather.windKmh)));
    }
  }

  static void renderCachedWeather()
  {
    WeatherData snapshot;
    bool hasData = false;

    portENTER_CRITICAL(&weatherMux);

    hasData = weatherAvailable;

    if (hasData)
    {
      snapshot = cachedWeather;
    }

    weatherDirty = false;

    portEXIT_CRITICAL(&weatherMux);

    if (hasData)
    {
      renderWeather(snapshot);
    }
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

    temperatureLabel = lv_label_create(scr);
    lv_label_set_text(temperatureLabel, "--°");

    lv_obj_set_width(temperatureLabel, 240);
    lv_obj_set_style_text_align(
        temperatureLabel,
        LV_TEXT_ALIGN_CENTER,
        0);

    lv_obj_set_style_text_color(
        temperatureLabel,
        lv_color_make(240, 245, 242),
        0);

    lv_obj_set_style_text_font(
        temperatureLabel,
        TEMP_FONT,
        0);

    lv_obj_set_style_text_letter_space(
        temperatureLabel,
        1,
        0);

    lv_obj_align(temperatureLabel, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t *accentLine = lv_obj_create(scr);
    makeStatic(accentLine);

    lv_obj_set_size(accentLine, 90, 2);

    lv_obj_set_style_bg_color(
        accentLine,
        lv_color_make(105, 125, 115),
        0);

    lv_obj_set_style_border_width(accentLine, 0, 0);
    lv_obj_set_style_radius(accentLine, 2, 0);

    lv_obj_align(accentLine, LV_ALIGN_CENTER, 0, 5);

    humidityValueLabel = createMetricCard(
        scr,
        -48,
        "HUMIDITY",
        "%");

    windValueLabel = createMetricCard(
        scr,
        48,
        "WIND",
        "km/h");

    renderCachedWeather();

    return scr;
  }

  bool updateWeather(const char *json)
  {
    if (json == nullptr || json[0] == '\0')
    {
      Serial.println("[Weather] Empty JSON");
      return false;
    }

    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, json);

    if (error)
    {
      Serial.print("[Weather] JSON error: ");
      Serial.println(error.c_str());
      return false;
    }

    if (!doc.is<JsonObject>())
    {
      Serial.println("[Weather] JSON root must be an object");
      return false;
    }

    WeatherData parsed;

    parsed.temperatureC = doc["temperature_c"] | NAN;
    parsed.humidityPercent = doc["humidity_percent"] | -1;
    parsed.windKmh = doc["wind_kmh"] | NAN;

    if (!isfinite(parsed.temperatureC) ||
        parsed.temperatureC < -100.0f ||
        parsed.temperatureC > 100.0f)
    {
      parsed.temperatureC = NAN;
    }

    if (parsed.humidityPercent < 0 ||
        parsed.humidityPercent > 100)
    {
      parsed.humidityPercent = -1;
    }

    if (!isfinite(parsed.windKmh) ||
        parsed.windKmh < 0.0f ||
        parsed.windKmh > 500.0f)
    {
      parsed.windKmh = NAN;
    }

    bool hasAnyUsableValue =
        isfinite(parsed.temperatureC) ||
        parsed.humidityPercent >= 0 ||
        isfinite(parsed.windKmh);

    if (!hasAnyUsableValue)
    {
      Serial.println("[Weather] No usable values in JSON");
      return false;
    }

    portENTER_CRITICAL(&weatherMux);

    cachedWeather = parsed;
    weatherAvailable = true;
    weatherDirty = true;

    portEXIT_CRITICAL(&weatherMux);

    Serial.printf(
        "[Weather] Temp: %.1f C | Humidity: %d%% | Wind: %.1f km/h\n",
        parsed.temperatureC,
        parsed.humidityPercent,
        parsed.windKmh);

    return true;
  }

  void update()
  {
    WeatherData snapshot;
    bool shouldRender = false;

    portENTER_CRITICAL(&weatherMux);

    if (weatherAvailable && weatherDirty)
    {
      snapshot = cachedWeather;
      weatherDirty = false;
      shouldRender = true;
    }

    portEXIT_CRITICAL(&weatherMux);

    if (shouldRender)
    {
      Serial.println("[Weather] rerendering weather");
      renderWeather(snapshot);
    }
  }

  void destroy(lv_obj_t *scr)
  {
    temperatureLabel = nullptr;
    humidityValueLabel = nullptr;
    windValueLabel = nullptr;

    lv_obj_delete(scr);
  }
}