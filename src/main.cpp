
/*    ___           ___                       ___                       ___
     /  /\         /__/\          ___        /  /\        ___          /__/|
    /  /::\       _\_ \:\        /  /\      /  /::\      /  /\        |  |:|
   /  /:/\:\     /__/\ \:\      /  /:/     /  /:/\:\    /  /:/        |  |:|
  /  /:/~/::\   _\_ \:\ \:\    /  /:/     /  /:/~/:/   /__/::\      __|__|:|
 /__/:/ /:/\:\ /__/\ \:\ \:\  /  /::\    /__/:/ /:/___ \__\/\:\__  /__/::::\____
 \  \:\/:/__\/ \  \:\ \:\/:/ /__/:/\:\   \  \:\/:::::/    \  \:\/\    ~\~~\::::/
  \  \::/       \  \:\ \::/  \__\/  \:\   \  \::/~~~~      \__\::/     |~~|:|~~
   \  \:\        \  \:\/:/        \  \:\   \  \:\          /__/:/      |  |:|
    \  \:\        \  \::/          \__\/    \  \:\         \__\/       |  |:|
     \__\/         \__\/                     \__\/                     |__|/

 ***************************************************************************
 *                                                                         *
 *   AWTRIX Light, a custom firmware for the Ulanzi clock                  *
 *                                                                         *
 *   Copyright (C) 2023  Stephan Mühl aka Blueforcer                       *
 *                                                                         *
 *   This work is licensed under a                                         *
 *   Creative Commons Attribution-NonCommercial-ShareAlike                 *
 *   4.0 International License.                                            *
 *                                                                         *
 *   More information:                                                     *
 *   https://github.com/Blueforcer/awtrix-light/blob/main/LICENSE.md       *
 *                                                                         *
 *   This firmware is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/

#include <Arduino.h>

#include "constant/Globals.h"
#include "manager/DisplayManager.h"
#include "manager/MQTTManager.h"
#include "manager/PeripheryManager.h"
#include "manager/ServerManager.h"
#include "manager/UpdateManager.h"

TaskHandle_t taskHandle;
volatile bool StopTask = false;

void BootAnimation(void *parameter)
{
  const TickType_t xDelay = 1 / portTICK_PERIOD_MS;
  while (true)
  {
    if (StopTask)
    {
      break;
    }
    DisplayManager.HSVtext(4, 6, "AWTRIX Ao", true, 0);
    vTaskDelay(xDelay);
  }
  vTaskDelete(NULL);
}

void setup()
{
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  delay(2000);
  loadSettings();
  Serial.begin(115200);
  PeripheryManager.setup();
  ServerManager.loadSettings();
  DisplayManager.setup();
  DisplayManager.HSVtext(9, 6, VERSION, true, 0);
  delay(500);
  xTaskCreatePinnedToCore(BootAnimation, "Task", 10000, NULL, 1, &taskHandle, 0);
  ServerManager.setup();
  // PeripheryManager.playBootSound();
  if (ServerManager.isConnected)
  {
    MQTTManager.setup();
    DisplayManager.loadNativeApps();
    UpdateManager.setup();
    DisplayManager.startArtnet();
    StopTask = true;
    float x = 4;
    while (x >= -85)
    {
      DisplayManager.HSVtext(x, 6, ("AWTRIX  Ao  " + ServerManager.myIP.toString()).c_str(), true, 0);
      x -= 0.18;
    }
  }
  else
  {
    AP_MODE = true;
    StopTask = true;
  }
  delay(200);
  DisplayManager.setBrightness(BRIGHTNESS);
}

void loop()
{
  ServerManager.tick();
  DisplayManager.tick();
  PeripheryManager.tick();
  if (ServerManager.isConnected)
  {
    MQTTManager.tick();
  }
}