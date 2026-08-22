//-------------------------------------------------------------------------------
// Copyright (c) 2024 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief Temperature logger for Raspberry Pi Pico

#include <cstdio>

#include "HWR/picoSense/Config.h"
#include "HWR/FilePortal.h"

#include "Display.h"

#define PRINTF if (0) printf

// --- E-PAPER -----------------------------------------------------------------

constexpr unsigned WIDTH  = HWR::Display::getWidth();
constexpr unsigned HEIGHT = HWR::Display::getHeight();

HWR::Display                display;
static HWR::Display::Canvas canvas;


// --- SENSOR ------------------------------------------------------------------

static HWR::TempSense temp_sensor;


// --- USB ---------------------------------------------------------------------

static HWR::FilePortal file_portal{"picoPlot",
                                   "https://github.com/SloeComputers/picoPlot"};

static HWR::UsbFile    usb{0x91C0, "picoPlot", file_portal};

extern "C" void IRQ_USBCTRL() { usb.irq(); }


// --- RTC ---------------------------------------------------------------------

static HWR::Rtc rtc;

extern "C" void IRQ_RTC() { rtc.irq(); }


// --- BUTTONS -----------------------------------------------------------------

static HWR::Buttons buttons{/* enable IRQ */ true};

extern "C" void IRQ_IO_BANK0() { buttons.irq(); }


// --- POWER -------------------------------------------------------------------

#if defined(HW_BADGER2040)

MTL::badger2040::Enable3V3  bat_latch{};
MTL::badger2040::EnableVRef enable_vref{};
MTL::badger2040::AdcVRef    vref;
MTL::badger2040::AdcVBat    vbat;

//! Return battery (mV)
unsigned getVBat()
{
   enable_vref = true;
   uint32_t vref_raw = vref;
   enable_vref = false;

   unsigned vbat_sense_millivolt = MTL::badger2040::VREF_MILLIVOLT * vbat / vref_raw;

   return vbat_sense_millivolt * MTL::badger2040::VBAT_SCALE;
}

#else

bool bat_latch{false};

static unsigned getVBat() { return 3300; }

#endif


//------------------------------------------------------------------------------

int main()
{
   file_portal.addREADME("picoPlot");

   bat_latch = true;

   PRINTF("picoPloT\n");

   temp_sensor.start();

   rtc.setDate(2026, 3, 8);
   rtc.setTime(17, 5, 0, 0);
   rtc.start();

   Display<WIDTH,HEIGHT> display(canvas);

   while(true)
   {
      // Current time NOTE previous rtc.sleep() will sample the time
      display.setDay(rtc.getDOTW(), rtc.getDay());
      display.setTime(rtc.getHour(), rtc.getMinute());

      signed temp = temp_sensor.read();
      display.setTemp(temp);

      unsigned vbat = getVBat();
      display.setVBat(vbat);

      PRINTF("DOW=%u DOM=%u\n", rtc.getDOTW(), rtc.getDay());
      PRINTF("%2u:%2u\n", rtc.getHour(), rtc.getMinute());
      PRINTF("%d.%u deg C\n", temp / 256, ((temp * 10) / 256) % 10);

      display.draw();

      // sleep for a minute since the last RTC event
      rtc.sleep(60);
   }

   return 0;
}
