#pragma once
#include <Wire.h>
#include <Arduino.h>
#include <lvgl.h>
#include <functional>

namespace TouchScreen
{
  inline std::function<void()> onSwipeUp;
  inline std::function<void()> onSwipeDown;
  inline std::function<void()> onSwipeLeft;
  inline std::function<void()> onSwipeRight;
  static bool _readTouchXY(int16_t &tx, int16_t &ty)
  {
    uint8_t d[5] = {0};
    uint8_t n = Wire.requestFrom((uint8_t)0x2E, (uint8_t)5);
    for (int i = 0; i < 5 && Wire.available(); i++)
      d[i] = Wire.read();
    while (Wire.available())
      Wire.read();

    if (n == 5 && d[0] == 0x01)
    {
      tx = d[2];
      ty = d[4];
      return true;
    }
    return false;
  }
  static void touchRead(lv_indev_t *indev, lv_indev_data_t *data)
  {
    static int16_t lastX = 120;
    static int16_t lastY = 120;
    static bool wasDown = false;
    static int16_t pressX = 0;
    static int16_t pressY = 0;

    int16_t tx, ty;
    if (TouchScreen::_readTouchXY(tx, ty))
    {
      if (!wasDown)
      {
        pressX = tx;
        pressY = ty;
        wasDown = true;
      }
      lastX = tx;
      lastY = ty;
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = lastX;
      data->point.y = lastY;
    }
    else
    {
      if (wasDown)
      {
        int16_t dx = lastX - pressX;
        int16_t dy = lastY - pressY;
        Serial.printf("Swipe  Δ=(%d,%d)  ", dx, dy);

        if (abs(dx) > abs(dy) && abs(dx) > 35)
        {
          if (dx > 0)
          {
            Serial.println("RIGHT");
            if (onSwipeRight)
              onSwipeRight();
          }
          else
          {
            Serial.println("LEFT");
            if (onSwipeLeft)
              onSwipeLeft();
          }
        }
        else if (abs(dy) > abs(dx) && abs(dy) > 35)
        {
          if (dy < 0)
          {
            Serial.println("UP");
            if (onSwipeUp)
              onSwipeUp();
          }
          else
          {
            Serial.println("DOWN");
            if (onSwipeDown)
              onSwipeDown();
          }
        }
        else
        {
          Serial.println("TAP");
        }
        wasDown = false;
      }
      data->state = LV_INDEV_STATE_RELEASED;
      data->point.x = lastX;
      data->point.y = lastY;
    }
  }

  static void createInstance()
  {
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchRead);
  }
};
