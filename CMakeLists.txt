cmake_minimum_required(VERSION 3.0.0)

project(v4l2-test VERSION 0.1.0)

# Library errno
add_library(errno
        STATIC
        src/errno/errno.cpp)
target_include_directories(errno PUBLIC include/errno)

# Library utils
add_library(utils
        STATIC
        src/utils/commandargs.cpp
        src/utils/commandargclass.cpp
        src/utils/framebuffer.cpp)
target_include_directories(utils PUBLIC include/utils)
target_link_libraries(utils PRIVATE errno v4l2)

# Library v4l2
add_library(v4l2
        STATIC
        src/v4l2/v4l2image.cpp
        src/v4l2/v4l2imagesource.cpp
        src/v4l2/v4l2imagesocket.cpp
        src/v4l2/v4l2autoexposure.cpp)
target_include_directories(v4l2 PUBLIC include/v4l2)
target_link_libraries(v4l2 PRIVATE errno)
target_link_libraries(v4l2 PRIVATE utils)
find_package(OpenMP)
if (OPENMP_FOUND)
        target_link_libraries(v4l2 PUBLIC OpenMP::OpenMP_CXX)
endif()
# set(OpenCV_DIR ~/Projects/opencv/platforms/linux/aarch64_4.1.1/install/lib/cmake/opencv4)
# find_package(OpenCV 4.1.1 REQUIRED)
# if (OPENCV_FOUND)
#         include_directories(${OpenCV_INCLUDE_DIRS})
#         target_link_libraries(v4l2 PUBLIC ${OpenCV_LIBS})
# endif()


# Executable v4l2-test
add_executable(v4l2-test 
        src/v4l2-test/streamtest.cpp
        src/v4l2-test/isptest.cpp
        src/v4l2-test/noisetest.cpp
        src/v4l2-test/v4l2-test.cpp)
target_link_libraries(v4l2-test PRIVATE utils v4l2)
add_custom_command(
        TARGET v4l2-test POST_BUILD 
        COMMAND scp ARGS v4l2-test vc@nvidia:/home/vc/test
        VERBATIM)

# Executable v4l2-server
add_executable(v4l2-server
        src/v4l2-server/v4l2-server.cpp)
target_link_libraries(v4l2-server PRIVATE utils v4l2)