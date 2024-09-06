
cmake_minimum_required(VERSION 3.16)
set(CMAKE_BUILD_TYPE Debug)

 
add_subdirectory(e_silero)

add_library(zvad SHARED zvad.cpp)

link_directories(${ONNXRUNTIME_ROOTDIR}/lib)
target_include_directories(zvad PRIVATE ${PROJECT_SOURCE_DIR}/include )
target_link_libraries(zvad PRIVATE e_silero)

add_subdirectory(c_example)

 
 

#copy_ort_dlls(fns_candy_style_transfer)
