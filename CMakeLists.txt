project(URC2014)
cmake_minimum_required(VERSION 2.6)

find_package(RoboCRISP REQUIRED)
include_directories(${RoboCRISP_INCLUDE_DIRS})


add_definitions(
  "-std=c++11"
  "-Wno-packed"
  "-Wno-conversion"
  "-Wno-attributes"
  )

if("${CMAKE_CXX_COMPILER}" MATCHES "clang\\+\\+")
  add_definitions(
    "-Wno-c++98-compat"
    "-Wno-c++98-compat-pedantic"
    "-Wno-disabled-macro-expansion"
    "-Wno-global-constructors"

    "-fno-spell-checking"
    )
else("${CMAKE_CXX_COMPILER}" MATCHES "clang\\+\\+")
  # Assuming g++.
  add_definitions(
    "-Wno-packed-bitfield-compat"
    )
endif("${CMAKE_CXX_COMPILER}" MATCHES "clang\\+\\+")



add_subdirectory("tests/control-test")
