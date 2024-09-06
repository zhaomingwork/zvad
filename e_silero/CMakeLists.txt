 

set(CMAKE_BUILD_TYPE Debug)
add_library(e_silero SHARED silero_engine.cpp)
link_directories(${ONNXRUNTIME_ROOTDIR}/lib)
target_include_directories(e_silero PRIVATE ${ONNXRUNTIME_ROOTDIR}/include ${PROJECT_SOURCE_DIR}/include )
target_link_libraries(e_silero PRIVATE ${ONNXRUNTIME_ROOTDIR}/lib/libonnxruntime.so )

 

 
 

#copy_ort_dlls(fns_candy_style_transfer)
