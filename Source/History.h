//-------------------------------------------------------------------------------
// Copyright (c) 2024 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief

#pragma once

#include <cassert>
#include <cstddef>
#include <limits>

template <typename TYPE,unsigned SIZE>
class History
{
public:
   History() = default;

   void push(const TYPE& value)
   {
      assert((head < BUF_SIZE) && (tail < BUF_SIZE));

      buffer[head++] = value;

      if (head == BUF_SIZE)
         head = 0;

      if (head == tail)
         if (++tail == BUF_SIZE)
            tail = 0;
   }

   size_t size() const
   {
      return head >= tail ? head - tail
                          : head + BUF_SIZE - tail;
   }

   bool empty() const { return size() == 0; }

   const TYPE& at(size_t index) const
   {
      assert(index < SIZE);

      size_t first = head > 0 ? head - 1 : BUF_SIZE - 1;

      return first >= index ? buffer[first - index]
                            : buffer[first + BUF_SIZE - index];
   }

   TYPE& at(size_t index)
   {
      assert(index < SIZE);

      size_t first = head > 0 ? head - 1 : BUF_SIZE - 1;

      return first >= index ? buffer[first - index]
                            : buffer[first + BUF_SIZE - index];
   }

   const TYPE& operator[](size_t index) const { return at(index); }
   TYPE&       operator[](size_t index)       { return at(index); }

   TYPE min() const
   {
      TYPE lowest = std::numeric_limits<TYPE>::max();

      for(size_t i = 0; i < size(); ++i)
      {
         TYPE value = at(i);

         if (value < lowest)
            lowest = value;
      }

      return lowest;
   }

   TYPE max() const
   {
      TYPE highest = std::numeric_limits<TYPE>::lowest();

      for(size_t i = 0; i < size(); ++i)
      {
         TYPE value = at(i);

         if (value > highest)
            highest = value;
      }

      return highest;
   }

private:
   static const size_t BUF_SIZE = SIZE + 1;

   size_t head{0};
   size_t tail{0};
   TYPE   buffer[BUF_SIZE];
};

