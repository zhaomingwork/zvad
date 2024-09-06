 

set(CMAKE_BUILD_TYPE Debug)
 

add_executable(example example.c)
target_link_libraries(example PRIVATE ${PROJECT_SOURCE_DIR}/build/libzvad.so)

 
 

 
