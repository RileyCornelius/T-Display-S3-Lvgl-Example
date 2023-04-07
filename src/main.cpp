#include <Arduino.h>
#include "ui/lv_setup.h"
#include "pin_config.h"

void setup()
{
  lv_begin(); 
}

void loop()
{
  lv_handler();
}