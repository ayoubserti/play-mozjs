cmake_minimum_required (VERSION 3.1) 
project (Plya-mozjs)

###############################################################################
## Enable C and C++ languages.
###############################################################################
enable_language(C CXX)

###############################################################################
## Use C++ 17.
###############################################################################
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(PkgConfig REQUIRED)

#check MozJS module
pkg_check_modules(JS REQUIRED js)

add_executable (play-mozjs "main.cc")

message ("${JS_LIBRARIES}")
target_link_libraries(play-mozjs ${JS_LIBRARIES})
target_include_directories(play-mozjs  PUBLIC ${JS_INCLUDE_DIRS})

