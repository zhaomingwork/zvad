 

set(CMAKE_BUILD_TYPE Debug)
 

add_executable(example example.c)
target_link_libraries(example PRIVATE /mnt/nfsfile/gitcode/silero/silero-vad/examples/cpp/build/libzvad.so)

 
 

 
