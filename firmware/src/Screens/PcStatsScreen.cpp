#include "ScreenManager.h"

#include <Arduino.h>
#include <cmath>
#include "Config.h"

namespace PcStatsScreen
{
#if LV_FONT_MONTSERRAT_20
    static const lv_font_t *ARC_VALUE_FONT = &lv_font_montserrat_20;
#elif LV_FONT_MONTSERRAT_16
    static const lv_font_t *ARC_VALUE_FONT = &lv_font_montserrat_16;
#else
    static const lv_font_t *ARC_VALUE_FONT = &lv_font_montserrat_14;
#endif

#if LV_FONT_MONTSERRAT_14
    static const lv_font_t *VALUE_FONT = &lv_font_montserrat_14;
#else
    static const lv_font_t *VALUE_FONT = &lv_font_montserrat_12;
#endif

    static constexpr int ARC_SIZE = 90;
    static constexpr int CARD_WIDTH = 80;
    static constexpr int CARD_HEIGHT = 46;

    static lv_obj_t *cpuArc = nullptr;
    static lv_obj_t *cpuLoadLabel = nullptr;
    static lv_obj_t *gpuArc = nullptr;
    static lv_obj_t *gpuLoadLabel = nullptr;

    static lv_obj_t *cpuTempLabel = nullptr;
    static lv_obj_t *cpuUnitLabel = nullptr;
    static lv_obj_t *gpuTempLabel = nullptr;
    static lv_obj_t *gpuUnitLabel = nullptr;

    static void createStatusElement(
        lv_obj_t *scr,
        int xOffset,
        const char *title,
        lv_obj_t **outArc,
        lv_obj_t **outValueLabel)
    {
        lv_obj_t *arc = lv_arc_create(scr);
        makeStatic(arc);

        lv_obj_set_size(arc, ARC_SIZE, ARC_SIZE);
        lv_obj_align(arc, LV_ALIGN_CENTER, xOffset, -20);

        lv_arc_set_rotation(arc, 135);
        lv_arc_set_bg_angles(arc, 0, 270);
        lv_arc_set_range(arc, 0, 100);
        lv_arc_set_value(arc, 0);

        lv_obj_remove_style(arc, nullptr, LV_PART_KNOB);

        lv_obj_set_style_arc_width(arc, 6, 0);
        lv_obj_set_style_arc_width(arc, 6, LV_PART_INDICATOR);

        lv_obj_set_style_arc_rounded(arc, true, 0);
        lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);

        lv_obj_set_style_arc_color(arc, lv_color_make(55, 65, 60), 0);
        lv_obj_set_style_arc_color(arc, lv_color_make(184, 204, 184), LV_PART_INDICATOR);

        lv_obj_t *titleLabel = lv_label_create(arc);
        lv_label_set_text(titleLabel, title);
        lv_obj_set_style_text_color(titleLabel, lv_color_make(135, 155, 145), 0);
        lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_letter_space(titleLabel, 1, 0);
        lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, -16);

        lv_obj_t *valueLabel = lv_label_create(arc);
        lv_label_set_text(valueLabel, "--%");
        lv_obj_set_style_text_color(valueLabel, lv_color_make(240, 245, 242), 0);
        lv_obj_set_style_text_font(valueLabel, ARC_VALUE_FONT, 0);
        lv_obj_align(valueLabel, LV_ALIGN_CENTER, 0, 2);

        *outArc = arc;
        *outValueLabel = valueLabel;
    }

    static void createMetricCard(
        lv_obj_t *scr,
        int xOffset,
        const char *title,
        const char *unit,
        lv_obj_t **outValueLabel,
        lv_obj_t **outUnitLabel)
    {
        lv_obj_t *card = lv_obj_create(scr);
        makeStatic(card);

        lv_obj_set_size(card, CARD_WIDTH, CARD_HEIGHT);
        lv_obj_set_style_bg_color(card, lv_color_make(25, 30, 28), 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, lv_color_make(55, 65, 60), 0);
        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_style_pad_all(card, 0, 0);
        lv_obj_align(card, LV_ALIGN_CENTER, xOffset, 60);

        lv_obj_t *titleLabel = lv_label_create(card);
        lv_label_set_text(titleLabel, title);
        lv_obj_set_width(titleLabel, CARD_WIDTH - 8);
        lv_obj_set_style_text_align(titleLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(titleLabel, lv_color_make(135, 155, 145), 0);
        lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_letter_space(titleLabel, 1, 0);
        lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 5);

        lv_obj_t *valueLabel = lv_label_create(card);
        lv_label_set_text(valueLabel, "--");
        lv_obj_set_width(valueLabel, CARD_WIDTH - 8);
        lv_obj_set_style_text_align(valueLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(valueLabel, lv_color_make(240, 245, 242), 0);
        lv_obj_set_style_text_font(valueLabel, VALUE_FONT, 0);
        lv_obj_align(valueLabel, LV_ALIGN_CENTER, 0, 2);

        lv_obj_t *unitLabel = lv_label_create(card);
        lv_label_set_text(unitLabel, unit);
        lv_obj_set_style_text_color(unitLabel, lv_color_make(105, 125, 115), 0);
        lv_obj_set_style_text_font(unitLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(unitLabel, LV_ALIGN_BOTTOM_MID, 0, -3);

        *outValueLabel = valueLabel;
        *outUnitLabel = unitLabel;
    }

    lv_obj_t *create()
    {
        lv_obj_t *scr = lv_obj_create(nullptr);
        makeStatic(scr);
        createTitle(scr, "PC STATS");

        lv_obj_set_style_bg_opa(scr, LV_OPA_0, 0);
        lv_obj_set_style_border_width(scr, 0, 0);
        lv_obj_set_style_pad_all(scr, 0, 0);

        createStatusElement(scr, -48, "CPU", &cpuArc, &cpuLoadLabel);
        createStatusElement(scr, 48, "GPU", &gpuArc, &gpuLoadLabel);

        lv_obj_t *accentLine = lv_obj_create(scr);
        makeStatic(accentLine);
        lv_obj_set_size(accentLine, 90, 2);
        lv_obj_set_style_bg_color(accentLine, lv_color_make(105, 125, 115), 0);
        lv_obj_set_style_border_width(accentLine, 0, 0);
        lv_obj_set_style_radius(accentLine, 2, 0);
        lv_obj_align(accentLine, LV_ALIGN_CENTER, 0, 31);

        const char *unit = Config::data.useCelsius ? "°C" : "°F";

        createMetricCard(scr, -45, "CPU TEMP", unit, &cpuTempLabel, &cpuUnitLabel);
        createMetricCard(scr, 45, "GPU TEMP", unit, &gpuTempLabel, &gpuUnitLabel);

        return scr;
    }

    void update() {}

    void destroy(lv_obj_t *scr)
    {
        cpuArc = nullptr;
        cpuLoadLabel = nullptr;
        gpuArc = nullptr;
        gpuLoadLabel = nullptr;
        cpuTempLabel = nullptr;
        cpuUnitLabel = nullptr;
        gpuTempLabel = nullptr;
        gpuUnitLabel = nullptr;

        lv_obj_delete(scr);
    }

    void setGpuTelemetry(float load, float temperature)
    {
        if (!gpuArc || !gpuLoadLabel || !gpuTempLabel || !gpuUnitLabel)
            return;

        int loadInt = static_cast<int>(std::round(load));
        lv_arc_set_value(gpuArc, loadInt);
        lv_label_set_text_fmt(gpuLoadLabel, "%d%%", loadInt);

        float temp = Config::data.useCelsius
                         ? temperature
                         : temperature * 9.0f / 5.0f + 32.0f;
        int tempInt = static_cast<int>(std::round(temp));

        lv_label_set_text_fmt(gpuTempLabel, "%d", tempInt);
        lv_label_set_text(gpuUnitLabel, Config::data.useCelsius ? "°C" : "°F");
    }

    void setCpuTelemetry(float load, float temperature)
    {
        if (!cpuArc || !cpuLoadLabel || !cpuTempLabel || !cpuUnitLabel)
            return;

        int loadInt = static_cast<int>(std::round(load));
        lv_arc_set_value(cpuArc, loadInt);
        lv_label_set_text_fmt(cpuLoadLabel, "%d%%", loadInt);

        float temp = Config::data.useCelsius
                         ? temperature
                         : temperature * 9.0f / 5.0f + 32.0f;
        int tempInt = static_cast<int>(std::round(temp));

        lv_label_set_text_fmt(cpuTempLabel, "%d", tempInt);
        lv_label_set_text(cpuUnitLabel, Config::data.useCelsius ? "°C" : "°F");
    }
}