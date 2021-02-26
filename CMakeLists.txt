cmake_minimum_required(VERSION 3.16)
# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
project(aoclib C)
set(CMAKE_C_STANDARD 99)
add_library(aoclib SHARED library.c library.h)
unset(PCRE2_LIBRARY CACHE)
find_package(PCRE2  10.30  REQUIRED)
include_directories(${PCRE2_INCLUDE_DIRS})
target_link_libraries(aoclib ${PCRE2_LIBRARIES})

include_directories(SYSTEM /usr/include/tcl)
target_link_libraries(aoclib tclstub8.6)




add_definitions(-DUSE_TCL_STUBS)