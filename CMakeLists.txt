
cmake_minimum_required (VERSION 3.16)



project(memory_test)
set(CMAKE_BUILD_TYPE Release ...)
set(CMAKE_CXX_FLAGS "-O3")  
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") 
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")

set(CMAKE_C_FLAGS "-O3")  
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") 
set(CMAKE_C_FLAGS_DEBUG "-O3 -g")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(src SCR_LIST)

include_directories(include)

add_executable(memory_test ${SCR_LIST})


find_package(Threads REQUIRED)
target_link_libraries(memory_test PRIVATE Threads::Threads)

