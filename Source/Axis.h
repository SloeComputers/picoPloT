//-------------------------------------------------------------------------------
// Copyright (c) 2024 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief State for a plot axis

#pragma once

class Axis
{
public:
   Axis(unsigned pos_min_, unsigned pos_max_)
      : pos_min(pos_min_)
      , pos_max(pos_max_)
   {
   }

   unsigned getMinPos() const { return pos_min; }
   unsigned getMaxPos() const { return pos_max; }

   signed getMinVal() const { return val_min; }
   signed getMaxVal() const { return val_max; }

   void setRange(signed min_, signed max_)
   {
      val_min  = min_;
      val_max  = max_;
      scale_fp = ((signed(pos_max) - signed(pos_min)) << FIXP_SHIFT) / (val_max - val_min);
   }

   //! Convert a value to a pixel position
   unsigned getPos(signed value_) const
   {
      return pos_min + (((value_ - val_min) * scale_fp) >> FIXP_SHIFT);
   }

private:
   static const unsigned FIXP_SHIFT = 16;

   const unsigned pos_min;    //!< Minimum pixel position
   const unsigned pos_max;    //!< Maximum pixel position
   signed         val_min{};  //!< Minimum value
   signed         val_max{};  //!< Maximum value
   signed         scale_fp{};
};

