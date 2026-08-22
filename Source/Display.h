//-------------------------------------------------------------------------------
// Copyright (c) 2024 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief

#pragma once

#include <cstdio>

#include "GUI/Canvas.h"
#include "GUI/Font/Teletext.h"

#include "Axis.h"
#include "History.h"

template <unsigned WIDTH, unsigned HEIGHT>
class Display
{
public:
   Display(GUI::Canvas& canvas_)
      : canvas(canvas_)
   {
      time_scale.setRange(-HIST_HOURS * 60, 0);
   }

   //! Update the current day of the week and day of the month
   void setDay(unsigned dow_, unsigned dom_)
   {
      if(dow_ != cur_dow)
      {
         new_day = true;
      }

      cur_dow = dow_;
      cur_dom = dom_;
   }

   //! Update the current time
   void setTime(unsigned hours_, unsigned mins_)
   {
      cur_hours = hours_;
      cur_mins  = mins_;
   }

   //! Update the current temperature
   void setTemp(signed value_)
   {
      cur_temp = (value_ * 10) / 256;

      total_temp += cur_temp;
      num_temp++;
   }

   void setVBat(unsigned vbat_)
   {
      vbat = vbat_;
   }

   //! Redraw
   void draw()
   {
      bool quick = true;

      if (draw_cycle-- == 0)
      {
         draw_cycle = MINS_PER_PIXEL - 1;
         quick      = false;

         // Record average temperature in the history
         signed avg_temp = total_temp / signed(num_temp);
         recordTemp(avg_temp);

         num_temp   = 0;
         total_temp = 0;
      }

      canvas.clear(WHITE);

      dispTemp(WIDTH - 82, 2);
      dispTime(WIDTH - 60, 94);
      //dispVBat(WIDTH - 88, 110);
      drawMainPlot();
      drawSubPlot();

      if (quick)
      {
         canvas.quickRefresh();
      }
      else
      {
         canvas.refresh();
      }
   }

private:
   void recordTemp(signed value_)
   {
      history_temp.push(value_);

      signed max_temp = history_temp.max();
      signed min_temp = history_temp.min();

      if (not history_max_temp.empty())
      {
         history_max_temp[0] = max_temp;
         history_min_temp[0] = min_temp;
      }

      if (new_day)
      {
         new_day = false;

         history_max_temp.push(max_temp);
         history_min_temp.push(min_temp);
      }
   }

   void printText(unsigned x, unsigned y, const GUI::Font* font, const char* text)
   {
      canvas.drawText(BLACK, WHITE, x, y, font, text);
   }

   void printTemp(unsigned         x,
                  unsigned         y,
                  const GUI::Font* font,
                  signed           value,
                  bool             decimal,
                  bool             units)
   {
      char text[8];
      char* s = text;

      signed degs = value / 10;

      if (abs(degs) < 10)
      {
         *s++ = ' ';
      }

      if (value < 0)
      {
         *s++ = '-';
         degs = -degs;
      }
      else
         *s++ = ' ';

      if (abs(degs) >= 10)
      {
         *s++ = '0' + (degs / 10);
      }

      *s++ = '0' + (degs % 10);

      if (decimal)
      {
         unsigned tenths = abs(value) % 10;

         *s++ = '.';
         *s++ = '0' + tenths;
      }

      if (units)
      {
         *s++ = 0x7F;
         *s++ = 'C';
      }

      *s = '\0';

      printText(x, y, font, text);
   }

   void dispTemp(unsigned x, unsigned y)
   {
      printTemp(x, y, &GUI::font_teletext18, cur_temp,
                /* decimal */ true, /* units */ true);
   }

   void dispTime(unsigned x, unsigned y)
   {
      static const char* dow[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
      printText(x, y, &GUI::font_teletext15, dow[cur_dow]);

      char text[10];
      snprintf(text, sizeof(text), "%2u", cur_dom);
      printText(x + 39, y, &GUI::font_teletext15, text);

      snprintf(text, sizeof(text), "%02u:%02u", cur_hours, cur_mins);
      printText(x, y + 18, &GUI::font_teletext18, text);
   }

   void dispVBat(unsigned x, unsigned y)
   {
      char text[10];
      snprintf(text, sizeof(text), "%4u", vbat);
      printText(x, y, &GUI::font_teletext9, text);

      printText(x, y + 9, &GUI::font_teletext9, "mV");
   }

   static signed roundDn(signed value, signed unit)
   {
      if (value < 0)
         value -= unit - 1;

      return (value / unit) * unit;
   }

   static signed roundUp(signed value, signed unit)
   {
      if (value > 0)
         value += unit - 1;

      return (value / unit) * unit;
   }

   void drawVertScale()
   {
      canvas.drawLine(BLACK,
                      MAIN_PLOT_X_LFT, temp_scale.getMinPos(),
                      MAIN_PLOT_X_LFT, temp_scale.getMaxPos());

      canvas.drawLine(BLACK,
                      MAIN_PLOT_X_RGT, temp_scale.getMaxPos(),
                      MAIN_PLOT_X_RGT, temp_scale.getMinPos());

      // Range is 1/2 deg below minimum to 1/2 deg above the maximum
      signed min_temp = history_temp.min() - 5;
      signed max_temp = history_temp.max() + 5;
      temp_scale.setRange(min_temp, max_temp);

      unsigned grid = (max_temp - min_temp) < 100 ? 10 : 50;
      signed   min  = roundUp(min_temp, grid);
      signed   max  = roundDn(max_temp, grid);

      for(signed value = min; value <= max; value += grid)
      {
         unsigned y = temp_scale.getPos(value);

         printTemp(MAIN_PLOT_X_LFT - 20, y - 3, &GUI::font_teletext9, value,
                   /* decimal */ false, /* uniyts */ false);

         for(unsigned x = MAIN_PLOT_X_LFT + 5; x < MAIN_PLOT_X_RGT; x+= 5)
            canvas.drawPoint(BLACK, x, y);

         canvas.drawLine(BLACK, MAIN_PLOT_X_LFT, y, MAIN_PLOT_X_LFT - 3, y);
         canvas.drawLine(BLACK, MAIN_PLOT_X_RGT, y, MAIN_PLOT_X_RGT + 3, y);
      }
   }

   void drawHorzScale()
   {
      canvas.drawLine(BLACK,
                      time_scale.getMaxPos(), MAIN_PLOT_Y_BTM,
                      time_scale.getMinPos(), MAIN_PLOT_Y_BTM);

      canvas.drawLine(BLACK,
                      time_scale.getMinPos(), MAIN_PLOT_Y_TOP,
                      time_scale.getMaxPos(), MAIN_PLOT_Y_TOP);

      unsigned cur_time = cur_hours * 60 + cur_mins;

      for(unsigned tick_time = 0; tick_time < HIST_HOURS * 60; tick_time += 60)
      {
         signed mins_in_past = cur_time - tick_time;
         if (mins_in_past < 0)
            mins_in_past += 24 * 60;

         unsigned x = time_scale.getPos(-mins_in_past);
         unsigned tick_len;

         if((tick_time % 180) == 0)
         {
            tick_len = 3;

            char text[3];
            snprintf(text, sizeof(text), "%2d", tick_time / 60);
            printText(x - 4, MAIN_PLOT_Y_BTM + 3, &GUI::font_teletext9, text);

            for(unsigned y = MAIN_PLOT_Y_TOP + 5; y < MAIN_PLOT_Y_BTM; y += 5)
               canvas.drawPoint(BLACK, x, y);
         }
         else
         {
            tick_len = 2;
         }

         canvas.drawLine(BLACK, x, MAIN_PLOT_Y_BTM, x, MAIN_PLOT_Y_BTM + tick_len);
         canvas.drawLine(BLACK, x, MAIN_PLOT_Y_TOP, x, MAIN_PLOT_Y_TOP - tick_len);
      }
   }

   void drawPoint(unsigned x, unsigned y)
   {
      canvas.drawPoint(BLACK, x, y);
      canvas.drawPoint(BLACK, x + 1, y);
      canvas.drawPoint(BLACK, x - 1, y);
      canvas.drawPoint(BLACK, x, y + 1);
      canvas.drawPoint(BLACK, x, y - 1);
   }

   void drawMainPlot()
   {
      drawVertScale();
      drawHorzScale();

      for(signed i = 0; i < signed(history_temp.size()); i++)
      {
         unsigned x = time_scale.getPos(-i * MINS_PER_PIXEL);
         unsigned y = temp_scale.getPos(history_temp[i]);

         drawPoint(x, y);
      }
   }

   void drawSubPlot()
   {
      signed wk_min = 1000;
      signed wk_max = 0;

      for(unsigned day = 0; day < history_max_temp.size(); day++)
      {
         signed max = history_max_temp[day];
         signed min = history_min_temp[day];

         if (max > wk_max)
            wk_max = max;

         if (min < wk_min)
            wk_min = min;
      }

      wk_temp_scale.setRange(wk_min, wk_max);

      static const char dow_letter[7] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};

      for(unsigned day = 0; day < history_max_temp.size(); day++)
      {
         unsigned x     = SUB_PLOT_X_RGT - day * 11 - 4;
         unsigned max_y = wk_temp_scale.getPos(history_max_temp[day]);
         unsigned min_y = wk_temp_scale.getPos(history_min_temp[day]);

         canvas.fillRect(BLACK, x - 5, max_y, x + 4, min_y);

         unsigned dow_y = ((max_y + min_y) / 2) - 4;
         signed dow   = cur_dow - day;
         if (dow < 0)
            dow += 7;

         canvas.drawChar(WHITE, BLACK, x - 3, dow_y,
                         &GUI::font_teletext9, dow_letter[dow]);
      }

      signed max_temp = history_max_temp.max();
      printText(SUB_PLOT_X_LFT,      SUB_PLOT_Y_TOP - 10, &GUI::font_teletext9, "max");
      printTemp(SUB_PLOT_X_RGT - 42, SUB_PLOT_Y_TOP - 10, &GUI::font_teletext9, max_temp,
                /* decimal */ true, /* units */ true);

      canvas.drawLine(BLACK, SUB_PLOT_X_LFT, SUB_PLOT_Y_TOP,
                             SUB_PLOT_X_RGT, SUB_PLOT_Y_TOP);

      signed min_temp = history_min_temp.min();
      printText(SUB_PLOT_X_LFT,      SUB_PLOT_Y_BTM + 3, &GUI::font_teletext9, "min");
      printTemp(SUB_PLOT_X_RGT - 42, SUB_PLOT_Y_BTM + 3, &GUI::font_teletext9, min_temp,
                /* decimal */ true, /* units */ true);

      canvas.drawLine(BLACK, SUB_PLOT_X_LFT, SUB_PLOT_Y_BTM,
                             SUB_PLOT_X_RGT, SUB_PLOT_Y_BTM);
   }

   static const unsigned HIST_HOURS     = 24;
   static const unsigned MINS_PER_PIXEL = WIDTH >= 296 ? 8 : 10;
   static const unsigned SAMPLES        = HIST_HOURS * 60 / MINS_PER_PIXEL;

   static const unsigned MAIN_PLOT_X_LFT = 20;
   static const unsigned MAIN_PLOT_Y_TOP = 3;
   static const unsigned MAIN_PLOT_X_RGT = MAIN_PLOT_X_LFT + HIST_HOURS * 60 / MINS_PER_PIXEL;
   static const unsigned MAIN_PLOT_Y_BTM = 112;

   static const unsigned SUB_PLOT_X_LFT = WIDTH - 75;
   static const unsigned SUB_PLOT_Y_TOP = 32;
   static const unsigned SUB_PLOT_X_RGT = WIDTH;
   static const unsigned SUB_PLOT_Y_BTM = 75;

   static const STB::Colour WHITE = STB::RGB(0xC0, 0xC0, 0xC0);
   static const STB::Colour BLACK = STB::RGB(0x00, 0x00, 0x00);

   GUI::Canvas& canvas;

   unsigned cur_dow{};
   unsigned cur_dom{};
   unsigned cur_hours{};
   unsigned cur_mins{};
   bool     new_day{true};
   signed   cur_temp{};
   signed   total_temp{};
   unsigned num_temp{};
   unsigned draw_cycle{0};
   unsigned vbat{0};

   History<signed, SAMPLES> history_temp;
   History<signed, 7>       history_max_temp;
   History<signed, 7>       history_min_temp;

   Axis temp_scale{MAIN_PLOT_Y_BTM, MAIN_PLOT_Y_TOP};
   Axis time_scale{MAIN_PLOT_X_LFT, MAIN_PLOT_X_RGT};
   Axis wk_temp_scale{SUB_PLOT_Y_BTM, SUB_PLOT_Y_TOP};
};
