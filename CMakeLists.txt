cmake_minimum_required(VERSION 3.0.0)

project(v4l2-test VERSION 0.1.0)

add_library(errno
        STATIC
        src/errno/errno.cpp)
target_include_directories(errno PUBLIC include/errno)

add_library(v4l2
        STATIC
        src/v4l2/v4l2image.cpp
        src/v4l2/v4l2imagesource.cpp
        src/v4l2/v4l2imagesocket.cpp)
target_include_directories(v4l2 PUBLIC include/v4l2)
target_link_libraries(v4l2 PRIVATE errno)

add_library(utils
        STATIC
        src/utils/commandargs.cpp
        src/utils/framebuffer.cpp)
target_include_directories(utils PUBLIC include/utils)
target_link_libraries(utils PRIVATE errno v4l2)

add_executable(v4l2-test 
        src/v4l2-test/v4l2-test.cpp)
target_link_libraries(v4l2-test PRIVATE utils v4l2)

add_executable(v4l2-server
        src/v4l2-server/v4l2-server.cpp)
target_link_libraries(v4l2-server PRIVATE utils v4l2)

find_package(OpenMP)
if (OPENMP_FOUND)
        target_link_libraries(v4l2-test PRIVATE OpenMP::OpenMP_CXX)
endif()
