
cmake_minimum_required (VERSION 3.5)
project (kduc)

# Kakadu headers

find_path(KDU_INCLUDE_DIR kdu_args.h PATH_SUFFIXES kakadu)
if(NOT KDU_INCLUDE_DIR)
  message(FATAL_ERROR "Kakadu header files not found.")
endif()

include_directories(${KDU_INCLUDE_DIR})

# Kakadu libraries

find_library(KDU_LIBRARY kdu)
if(NOT KDU_LIBRARY)
  message(FATAL_ERROR "Kakadu library not found.")
endif()

find_library(KDU_AUX_LIBRARY kdu_aux)
if(NOT KDU_AUX_LIBRARY)
  message(FATAL_ERROR "Kakadu auxiliary library not found.")
endif()

# Wrapper library

include_directories(src/main/cpp)

add_library(kduc SHARED src/main/cpp/kduc.cpp)
target_link_libraries(kduc ${KDU_LIBRARY} ${KDU_AUX_LIBRARY} pthread ${CMAKE_DL_LIBS})

install(TARGETS kduc LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES src/main/cpp/kduc.h DESTINATION include)

# smoke tests

enable_testing()

include(CTest)

file(GLOB_RECURSE UNIT_TESTS src/test/cpp/*.cpp )
file(COPY "src/test/resources" DESTINATION "${CMAKE_BINARY_DIR}")

foreach(UNIT_TEST_PATH ${UNIT_TESTS} )
    get_filename_component(UNIT_TEST_NAME ${UNIT_TEST_PATH} NAME_WE)
    add_executable(${UNIT_TEST_NAME} ${UNIT_TEST_PATH} )
    target_link_libraries(${UNIT_TEST_NAME} kduc)
    add_test(${UNIT_TEST_NAME} ${UNIT_TEST_NAME} WORKINGDIRECTORY "${CMAKE_BINARY_DIR}")
endforeach(UNIT_TEST_PATH ${UNIT_TESTS})
