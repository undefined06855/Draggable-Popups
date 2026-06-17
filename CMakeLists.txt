cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(DraggablePopups VERSION 1.0.0)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE PLATFORM_SPECIFIC_SOURCES src/platform/*.cpp)
list(REMOVE_ITEM SOURCES ${PLATFORM_SPECIFIC_SOURCES})

if ((APPLE AND NOT IOS) OR WIN32)
    file(GLOB_RECURSE DESKTOP_SOURCES src/platform/desktop/*.cpp)
    list(APPEND SOURCES ${DESKTOP_SOURCES})
elseif (ANDROID)
    file(GLOB_RECURSE ANDROID_SOURCES src/platform/android/*.cpp)
    list(APPEND SOURCES ${ANDROID_SOURCES})
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
