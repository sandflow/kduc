
cmake_minimum_required (VERSION 3.5)
project (kduc)

# Kakadu SDK support

find_path(KDU_INCLUDE_DIR kdu_args.h PATH_SUFFIXES kakadu)
find_library(KDU_LIBRARY NAMES libkdu_a82R)

if(KDU_INCLUDE_DIR AND KDU_LIBRARY)
  set(KDU_PRESENT 1)
  message("Kakadu SDK found: ${KDU_LIBRARY} and ${KDU_INCLUDE_DIR}")
  include_directories(${KDU_INCLUDE_DIR})
else()
  message("Kakadu SDK not found.")
endif()

# Wrapper library

add_library(kduc src/main/cpp/kduc.cpp)
target_link_libraries(kduc ${KDU_LIBRARY})