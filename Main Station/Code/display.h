#ifndef DISPLAY_H
#define DISPLAY_H

#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_RGB _panel_instance;
  lgfx::Bus_RGB _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_GT911 _touch_instance;

public:
  LGFX(void);
};

extern LGFX lcd;

#endif