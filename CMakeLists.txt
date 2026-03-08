#-------------------------------------------------------------------------------
# Copyright (c) 2024 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(picoPloT
        LANGUAGES CXX C ASM
        VERSION 0.3)

include(CTest)

set(HARDWARE picoSense)

add_subdirectory(Platform/Hardware)
add_subdirectory(Source)
