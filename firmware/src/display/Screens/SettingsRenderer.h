#pragma once
#include <array>
#include <cstddef>
#include <lvgl.h>

namespace SettingsRenderer
{
  using Callback = void (*)(lv_event_t *event);

  enum ObjType
  {

    Tab,
    Switch,
    Page,
    Slider,
    Action,

  };

  struct SwitchConfig
  {
    const char *options[2];
    uint8_t selected;
  };

  struct SliderConfig
  {
    int32_t minValue;
    int32_t maxValue;
    int32_t value;
    const char *unit;
  };

  struct MenuObj
  {
    ObjType objType;
    const char *title;

    MenuObj *children;
    uint8_t childCount;
    Callback callback;
    MenuObj *page;
    SwitchConfig switchConfig;
    SliderConfig sliderConfig;

    constexpr MenuObj(ObjType type,
                      const char *itemTitle,
                      MenuObj *itemChildren = nullptr,
                      uint8_t itemChildCount = 0,
                      Callback itemCallback = nullptr,
                      MenuObj *itemPage = nullptr)
        : objType(type),
          title(itemTitle),
          children(itemChildren),
          childCount(itemChildCount),
          callback(itemCallback),
          page(itemPage),
          switchConfig{{nullptr, nullptr}, 0},
          sliderConfig{0, 0, 0, nullptr}
    {
    }
  };

  constexpr uint8_t MAX_PAGE_DEPTH = 4;

  struct NavigationState
  {
    MenuObj *pages[MAX_PAGE_DEPTH]{};
    uint8_t depth = 0;
  };

  inline MenuObj makeSwitch(const char *title,
                            const char *leftOption,
                            const char *rightOption,
                            uint8_t selected = 0,
                            Callback callback = nullptr)
  {
    MenuObj item(ObjType::Switch, title, nullptr, 0, callback);
    item.switchConfig = {{leftOption, rightOption}, selected};
    return item;
  }

  inline MenuObj makeSlider(const char *title,
                            int32_t minValue,
                            int32_t maxValue,
                            int32_t value,
                            const char *unit = nullptr,
                            Callback callback = nullptr)
  {
    MenuObj item(ObjType::Slider, title, nullptr, 0, callback);
    item.sliderConfig = {minValue, maxValue, value, unit};
    return item;
  }

  inline MenuObj makeAction(const char *title, Callback callback = nullptr)
  {
    return MenuObj(ObjType::Action, title, nullptr, 0, callback);
  }

  template <typename... Items>
  inline std::array<MenuObj, sizeof...(Items)> makeItems(const Items &...items)
  {
    return std::array<MenuObj, sizeof...(Items)>{{items...}};
  }

  template <std::size_t ItemCount>
  inline MenuObj makePage(const char *title,
                          std::array<MenuObj, ItemCount> &items)
  {
    static_assert(ItemCount <= UINT8_MAX, "A settings page has too many items");
    return MenuObj(ObjType::Page, title, items.data(),
                   static_cast<uint8_t>(ItemCount));
  }

  inline MenuObj makeTab(const char *title,
                         MenuObj &page,
                         Callback callback = nullptr)
  {
    return MenuObj(ObjType::Tab, title, nullptr, 0, callback, &page);
  }

  inline void renderPage(lv_obj_t *menu, MenuObj &pageObj);
  inline void createSettingsTab(lv_obj_t *menu, MenuObj *tabObj);
  inline void createSettingsSwitch(lv_obj_t *menu, MenuObj *switchObj);
  inline void createSettingsSlider(lv_obj_t *menu, MenuObj *sliderObj);
  inline void createSettingsAction(lv_obj_t *menu, MenuObj *actionObj);
  inline void createBackButton(lv_obj_t *menu);

  inline void renderPage(lv_obj_t *menu, MenuObj &pageObj)
  {
    lv_obj_clean(menu);

    NavigationState *navigation =
        static_cast<NavigationState *>(lv_obj_get_user_data(menu));
    if (navigation != nullptr && navigation->depth > 1)
    {
      createBackButton(menu);
    }

    if (pageObj.children == nullptr)
    {
      return;
    }

    for (uint8_t i = 0; i < pageObj.childCount; i++)
    {
      MenuObj &child = pageObj.children[i];

      switch (child.objType)
      {
      case ObjType::Tab:
        createSettingsTab(menu, &child);
        break;

      case ObjType::Switch:
        createSettingsSwitch(menu, &child);
        break;

      case ObjType::Slider:
        createSettingsSlider(menu, &child);
        break;

      case ObjType::Action:
        createSettingsAction(menu, &child);
        break;

      default:
        break;
      }
    }
  }
  inline void createSettingsTab(lv_obj_t *menu, MenuObj *tabObj)
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
    lv_label_set_text(tabLabel, tabObj->title);

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

    lv_obj_add_event_cb(
        tab,
        [](lv_event_t *e)
        {
          MenuObj *item =
              static_cast<MenuObj *>(lv_event_get_user_data(e));
          if (item == nullptr)
          {
            return;
          }

          if (item->page != nullptr)
          {
            lv_obj_t *tab =
                static_cast<lv_obj_t *>(lv_event_get_current_target(e));
            lv_obj_t *menu = lv_obj_get_parent(tab);

            NavigationState *navigation =
                static_cast<NavigationState *>(lv_obj_get_user_data(menu));
            if (navigation == nullptr || navigation->depth >= MAX_PAGE_DEPTH)
            {
              return;
            }

            navigation->pages[navigation->depth++] = item->page;
            renderPage(menu, *item->page);
            return;
          }

          if (item->callback != nullptr)
          {
            item->callback(e);
          }
        },
        LV_EVENT_CLICKED,
        tabObj);
  }

  inline lv_obj_t *createControlGroup(lv_obj_t *menu, const char *title,
                                      int32_t height)
  {
    lv_obj_t *group = lv_obj_create(menu);
    lv_obj_set_size(group, 184, height);
    lv_obj_set_style_bg_opa(group, 0, 0);
    lv_obj_set_style_border_width(group, 0, 0);
    lv_obj_set_style_pad_all(group, 0, 0);
    lv_obj_remove_flag(group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(group);
    lv_label_set_text_static(label, title != nullptr ? title : "");
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8D1CB), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 6, 0);
    return group;
  }

  inline void createSettingsSwitch(lv_obj_t *menu, MenuObj *switchObj)
  {
    SwitchConfig *config = &switchObj->switchConfig;
    if (config->options[0] == nullptr || config->options[1] == nullptr)
    {
      return;
    }

    lv_obj_t *group = createControlGroup(menu, switchObj->title, 54);
    if (config->selected > 1)
    {
      config->selected = 0;
    }

    lv_obj_t *leftLabel = lv_label_create(group);
    lv_label_set_text_static(leftLabel, config->options[0]);
    lv_obj_set_style_text_color(leftLabel, lv_color_hex(0x9CA8A1), 0);
    lv_obj_set_style_text_font(leftLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(leftLabel, LV_ALIGN_BOTTOM_LEFT, 12, -5);

    lv_obj_t *rightLabel = lv_label_create(group);
    lv_label_set_text_static(rightLabel, config->options[1]);
    lv_obj_set_style_text_color(rightLabel, lv_color_hex(0x9CA8A1), 0);
    lv_obj_set_style_text_font(rightLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(rightLabel, LV_ALIGN_BOTTOM_RIGHT, -12, -5);

    lv_obj_t *selector = lv_switch_create(group);
    lv_obj_set_size(selector, 58, 28);
    lv_obj_align(selector, LV_ALIGN_BOTTOM_MID, 0, 0);
    if (config->selected == 1)
    {
      lv_obj_add_state(selector, LV_STATE_CHECKED);
    }

    lv_obj_set_style_bg_color(selector, lv_color_hex(0x26312C), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(selector, 255, LV_PART_MAIN);
    lv_obj_set_style_border_width(selector, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(selector, lv_color_hex(0x47534D), LV_PART_MAIN);
    lv_obj_set_style_bg_color(selector, lv_color_hex(0x718C7A),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(selector, 255,
                            LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(selector, lv_color_hex(0xD8DFDA), LV_PART_KNOB);
    lv_obj_set_style_shadow_width(selector, 4, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(selector, 50, LV_PART_KNOB);

    lv_obj_add_event_cb(
        selector,
        [](lv_event_t *e)
        {
          MenuObj *item =
              static_cast<MenuObj *>(lv_event_get_user_data(e));
          lv_obj_t *selector =
              static_cast<lv_obj_t *>(lv_event_get_current_target(e));
          if (item == nullptr || selector == nullptr)
          {
            return;
          }

          item->switchConfig.selected =
              lv_obj_has_state(selector, LV_STATE_CHECKED) ? 1 : 0;

          if (item->callback != nullptr)
          {
            item->callback(e);
          }
        },
        LV_EVENT_VALUE_CHANGED,
        switchObj);
  }

  inline void updateSliderValueLabel(lv_obj_t *label,
                                     int32_t value,
                                     const char *unit)
  {
    if (unit != nullptr && unit[0] != '\0')
    {
      lv_label_set_text_fmt(label, "%ld %s", static_cast<long>(value), unit);
      return;
    }

    lv_label_set_text_fmt(label, "%ld", static_cast<long>(value));
  }

  inline void createSettingsSlider(lv_obj_t *menu, MenuObj *sliderObj)
  {
    SliderConfig *config = &sliderObj->sliderConfig;
    if (config->minValue > config->maxValue)
    {
      return;
    }

    if (config->value < config->minValue)
    {
      config->value = config->minValue;
    }
    else if (config->value > config->maxValue)
    {
      config->value = config->maxValue;
    }

    lv_obj_t *group = createControlGroup(menu, sliderObj->title, 54);

    lv_obj_t *valueLabel = lv_label_create(group);
    updateSliderValueLabel(valueLabel, config->value, config->unit);
    lv_obj_set_style_text_color(valueLabel, lv_color_hex(0xAAB7AF), 0);
    lv_obj_set_style_text_font(valueLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(valueLabel, LV_ALIGN_TOP_RIGHT, -6, 0);

    lv_obj_t *slider = lv_slider_create(group);
    lv_obj_set_user_data(slider, valueLabel);
    lv_slider_set_range(slider, config->minValue, config->maxValue);
    lv_slider_set_value(slider, config->value, LV_ANIM_OFF);
    lv_obj_set_size(slider, 160, 8);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -7);

    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2A342F), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider, 220, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x718C7A), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, 255, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, 4, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xD1D8D3), LV_PART_KNOB);
    lv_obj_set_style_width(slider, 16, LV_PART_KNOB);
    lv_obj_set_style_height(slider, 16, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(slider, 4, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(slider, 50, LV_PART_KNOB);

    lv_obj_add_event_cb(
        slider,
        [](lv_event_t *e)
        {
          MenuObj *item =
              static_cast<MenuObj *>(lv_event_get_user_data(e));
          lv_obj_t *slider =
              static_cast<lv_obj_t *>(lv_event_get_current_target(e));
          if (item == nullptr || slider == nullptr)
          {
            return;
          }

          item->sliderConfig.value = lv_slider_get_value(slider);

          lv_obj_t *valueLabel =
              static_cast<lv_obj_t *>(lv_obj_get_user_data(slider));
          if (valueLabel != nullptr)
          {
            updateSliderValueLabel(valueLabel,
                                   item->sliderConfig.value,
                                   item->sliderConfig.unit);
          }

        },
        LV_EVENT_VALUE_CHANGED,
        sliderObj);

    if (sliderObj->callback != nullptr)
    {
      lv_obj_add_event_cb(
          slider,
          [](lv_event_t *e)
          {
            MenuObj *item =
                static_cast<MenuObj *>(lv_event_get_user_data(e));
            if (item != nullptr && item->callback != nullptr)
            {
              item->callback(e);
            }
          },
          LV_EVENT_RELEASED,
          sliderObj);
    }
  }

  inline void createSettingsAction(lv_obj_t *menu, MenuObj *actionObj)
  {
    lv_obj_t *button = lv_button_create(menu);
    lv_obj_set_size(button, 184, 40);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x26332D), 0);
    lv_obj_set_style_bg_opa(button, 220, 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x718C7A), 0);
    lv_obj_set_style_border_opa(button, 180, 0);
    lv_obj_set_style_radius(button, 11, 0);
    lv_obj_set_style_pad_all(button, 0, 0);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text_static(label,
                             actionObj->title != nullptr ? actionObj->title : "");
    lv_obj_set_style_text_color(label, lv_color_hex(0xDCE4DE), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_center(label);

    if (actionObj->callback != nullptr)
    {
      lv_obj_add_event_cb(
          button,
          [](lv_event_t *e)
          {
            MenuObj *item =
                static_cast<MenuObj *>(lv_event_get_user_data(e));
            if (item != nullptr && item->callback != nullptr)
            {
              item->callback(e);
            }
          },
          LV_EVENT_CLICKED,
          actionObj);
    }
  }

  inline void createBackButton(lv_obj_t *menu)
  {
    lv_obj_t *button = lv_button_create(menu);

    lv_obj_set_size(button, 184, 40);

    lv_obj_set_style_bg_color(button, lv_color_hex(0x151B19), 0);
    lv_obj_set_style_bg_opa(button, 220, 0);

    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x37433D), 0);
    lv_obj_set_style_border_opa(button, 140, 0);

    lv_obj_set_style_radius(button, 13, 0);
    lv_obj_set_style_pad_all(button, 0, 0);

    lv_obj_set_style_shadow_width(button, 7, 0);
    lv_obj_set_style_shadow_opa(button, 50, 0);
    lv_obj_set_style_shadow_offset_y(button, 2, 0);

    lv_obj_t *accent = lv_obj_create(button);
    lv_obj_set_size(accent, 3, 16);
    lv_obj_set_style_radius(accent, 2, 0);
    lv_obj_set_style_bg_color(accent, lv_color_hex(0x718C7A), 0);
    lv_obj_set_style_bg_opa(accent, 180, 0);
    lv_obj_set_style_border_width(accent, 0, 0);
    lv_obj_align(accent, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text_static(label, "<  BACK");

    lv_obj_set_style_text_color(label, lv_color_hex(0xD1D8D3), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);

    lv_obj_align(label, LV_ALIGN_LEFT_MID, 26, 0);

    lv_obj_t *detail = lv_obj_create(button);
    lv_obj_set_size(detail, 16, 1);
    lv_obj_set_style_radius(detail, 1, 0);
    lv_obj_set_style_bg_color(detail, lv_color_hex(0x69766F), 0);
    lv_obj_set_style_bg_opa(detail, 100, 0);
    lv_obj_set_style_border_width(detail, 0, 0);
    lv_obj_align(detail, LV_ALIGN_RIGHT_MID, -14, 0);

    lv_obj_add_event_cb(
        button,
        [](lv_event_t *e)
        {
          lv_obj_t *button =
              static_cast<lv_obj_t *>(lv_event_get_current_target(e));

          lv_obj_t *menu = lv_obj_get_parent(button);

          NavigationState *navigation =
              static_cast<NavigationState *>(lv_obj_get_user_data(menu));

          if (navigation == nullptr || navigation->depth <= 1)
          {
            return;
          }

          --navigation->depth;

          MenuObj *previousPage =
              navigation->pages[navigation->depth - 1];

          navigation->pages[navigation->depth] = nullptr;

          if (previousPage != nullptr)
          {
            renderPage(menu, *previousPage);
          }
        },
        LV_EVENT_CLICKED,
        nullptr);
  }
  inline void renderObj(lv_obj_t *menu, MenuObj &obj,
                        NavigationState &navigation)
  {
    switch (obj.objType)
    {
    case ObjType::Page:
      navigation.depth = 1;
      navigation.pages[0] = &obj;
      lv_obj_set_user_data(menu, &navigation);
      renderPage(menu, obj);
      break;
    default:
      break;
    }
  }

  inline void render(lv_obj_t *menu, MenuObj &startPage)
  {
    static NavigationState navigation;
    renderObj(menu, startPage, navigation);
  }
}
