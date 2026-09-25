

#include <lvgl.h>
#include "ScreenManager.h"
#include "SettingsRenderer.h"
#include "Config.h"
#include "Callbacks.h"
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
        lv_obj_set_style_pad_left(menu, 10, 0);
        lv_obj_set_style_pad_right(menu, 10, 0);
        lv_obj_set_style_pad_bottom(menu, 10, 0);

        lv_obj_set_scroll_dir(menu, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_OFF);

        lv_obj_align(menu, LV_ALIGN_TOP_MID, 0, 58);
        createTitle(scr, "SETTINGS");

        static auto clockItems = SettingsRenderer::makeItems(
            SettingsRenderer::makeSwitch("Time format", "12 H", "24 H", Config::data.use24Hour, Callbacks::timeFormat_cb));
        static auto clockPage = SettingsRenderer::makePage("Clock", clockItems);

        static auto weatherItems = SettingsRenderer::makeItems(
            SettingsRenderer::makeSwitch("Temperature", "C", "F", !Config::data.useCelsius, Callbacks::temperatureUnit_cb),
            SettingsRenderer::makeSlider("Refresh rate", 5, 60, Config::data.weatherUpdateMin, "min", Callbacks::weatherRefreshRate_cb),
            SettingsRenderer::makeAction("Refresh now", Callbacks::refreshWeather_cb));
        static auto weatherPage = SettingsRenderer::makePage("Weather", weatherItems);

        static auto timeoutItems = SettingsRenderer::makeItems(
            SettingsRenderer::makeSlider("Brightness", 10, 100, Config::data.brightness, "%", Callbacks::brightness_cb),
            SettingsRenderer::makeSlider("Screen timeout", 5, 120, Config::data.timeoutSec == 0 ? 30 : Config::data.timeoutSec, "s", Callbacks::screenTimeout_cb),
            SettingsRenderer::makeSwitch("Always on", "Off", "On", Config::data.timeoutSec == 0, Callbacks::alwaysOn_cb));
        static auto timeoutPage = SettingsRenderer::makePage("Screen", timeoutItems);

        static auto systemItems = SettingsRenderer::makeItems(
            SettingsRenderer::makeSlider("Battery Refresh", 1, 60, Config::data.batteryUpdateMin, "min", Callbacks::batteryUpdateRate_cb),
            SettingsRenderer::makeAction("Restart Display", Callbacks::restartDisplay_cb));
        static auto systemPage = SettingsRenderer::makePage("System", systemItems);

        static auto displayItems = SettingsRenderer::makeItems(
            SettingsRenderer::makeTab("Clock", clockPage),
            SettingsRenderer::makeTab("Weather", weatherPage),
            SettingsRenderer::makeTab("Screen", timeoutPage),
            SettingsRenderer::makeTab("System", systemPage));
        static auto startPage = SettingsRenderer::makePage("Settings", displayItems);

        SettingsRenderer::render(menu, startPage);
        return scr;
    }

    void update() { /* static */ }

    void destroy(lv_obj_t *scr)
    {
        lv_obj_delete(scr);
    }
}
