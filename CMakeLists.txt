#-------------------------------------------------------------------------------
# Copyright (c) 2024 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(picoPloT
        LANGUAGES CXX C ASM
        VERSION 0.3)

if(NOT DEFINED BUILD_TESTING)
    set(BUILD_TESTING OFF CACHE BOOL "Build unit tests")
endif()
include(CTest)

set(HWR_CONFIG picoSense)

add_subdirectory(PDK)
add_subdirectory(Source)
