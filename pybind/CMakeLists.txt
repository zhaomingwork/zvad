# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

set(CMAKE_BUILD_TYPE Debug)
 
include(FetchContent)
 

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/pybind/pybind11 )
FetchContent_Declare(websocketpp
GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.13.6
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/pybind/pybind11
  )

FetchContent_MakeAvailable(websocketpp)
endif()

add_subdirectory(pybind11)
pybind11_add_module(py_zvad py_zvad.cpp)#重点
target_link_libraries(py_zvad PRIVATE ${PROJECT_SOURCE_DIR}/build/libzvad.so)
 
 

#copy_ort_dlls(fns_candy_style_transfer)
