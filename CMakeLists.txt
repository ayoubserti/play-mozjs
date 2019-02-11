cmake_minimum_required (VERSION 3.1) 
project (Plya-mozjs)

###############################################################################
## Enable C and C++ languages.
###############################################################################
enable_language(C CXX)

###############################################################################
## Use C++ 17.
###############################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(USE_STANDALONE_ASIO on)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory("Simple-WebSocket-Server")

find_package(PkgConfig REQUIRED)

link_directories("/usr/local/lib" "/usr/local/opt/openssl/lib")

#check MozJS module
pkg_check_modules(JS REQUIRED js)

add_executable (play-mozjs "main.cc" "websocket.cc" "websocket.h" "debugger-wrap.h" "debugger-wrap.cc" "utils.h" "utils.cc" "scopeJs.h")

message ("${JS_LIBRARIES}")
target_link_libraries(play-mozjs ${JS_LIBRARIES} ssl crypto) 
target_include_directories(play-mozjs  PUBLIC ${JS_INCLUDE_DIRS} Simple-WebSocket-Server/  "/usr/local/opt/openssl/include" "/usr/local/opt/asio/include")

add_dependencies(play-mozjs  simple-websocket-server)

add_custom_command(
        TARGET play-mozjs
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/debugger-script.js"
                "${CMAKE_CURRENT_BINARY_DIR}/debugger-script.js"
        
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/script.js"
                "${CMAKE_CURRENT_BINARY_DIR}/script.js"        
                )


