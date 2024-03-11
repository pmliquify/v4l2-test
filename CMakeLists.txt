cmake_minimum_required(VERSION 3.5.0)

project(V4L2Test 
        VERSION 0.3.0
        DESCRIPTION "V4L2 testing application"
        HOMEPAGE_URL "https://github.com/pmliquify/v4l2-test"
        LANGUAGES CXX)
configure_file (
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_BINARY_DIR}/include/version.h"
)


# --- Packages ----------------------------------------------------------------

find_package(OpenMP)

# set(OpenCV_DIR ~/libs/opencv/4.6.0/lib/cmake/opencv4)
# find_package(OpenCV 4.1.1 REQUIRED)
# if (OPENCV_FOUND)
#         include_directories(${OpenCV_INCLUDE_DIRS})
# endif()

# --- Librarys ----------------------------------------------------------------

# Library utils
add_library(utils
        STATIC
        src/utils/errno.cpp
        src/utils/commandargs.cpp
        src/utils/commandargsconsumer.cpp)
target_include_directories(utils PUBLIC include)

# Library cv
add_library(cv
        STATIC
        src/cv/image.cpp
        src/cv/imageprint.cpp
        src/cv/imagestats.cpp)
target_include_directories(cv PUBLIC include)
if (OPENMP_FOUND)
        target_link_libraries(cv PUBLIC OpenMP::OpenMP_CXX)
endif()

# Library network
add_library(network
        STATIC
        src/network/imagesocket.cpp)
target_include_directories(network PUBLIC include)
target_link_libraries(network PRIVATE cv)

# Library sources
add_library(sources
        STATIC
        src/sources/v4l2image.cpp
        src/sources/v4l2imagesource.cpp
        src/sources/socketserversource.cpp
        src/sources/autoexposure.cpp)
target_include_directories(sources PUBLIC include)
target_link_libraries(sources PRIVATE cv utils)
if (OPENCV_FOUND)
        target_link_libraries(sources PUBLIC ${OpenCV_LIBS})
endif()

# Library gui
add_library(gui
        STATIC
        src/gui/framebuffer.cpp)
target_include_directories(gui PUBLIC include)
target_link_libraries(gui PRIVATE cv sources)
if (OPENCV_FOUND)
        target_sources(gui PRIVATE src/gui/opencv/viewer.cpp)
        target_link_libraries(gui PUBLIC ${OpenCV_LIBS})
else()
        target_sources(gui PRIVATE src/gui/viewer.cpp)
endif()

# Library runner
add_library(runner
        STATIC
        src/runners/basicstreamrunner.cpp
        src/runners/streamrunner.cpp
        src/runners/isprunner.cpp
        src/runners/noisetestrunner.cpp
        src/runners/socketclientrunner.cpp)
target_include_directories(runner PUBLIC include)
target_link_libraries(runner PRIVATE cv utils network gui sources)


# --- Executables -------------------------------------------------------------

# Executable v4l2-test
add_executable(v4l2-test 
        src/v4l2-test/main.cpp)
target_include_directories(v4l2-test PRIVATE ${PROJECT_BINARY_DIR}/include)
target_link_libraries(v4l2-test PRIVATE cv utils network gui sources runner)

# add_custom_command(
#         TARGET v4l2-test POST_BUILD 
#         COMMAND scp ARGS v4l2-test root@imx8mp-var-dart:/home/root
#         VERBATIM)


# --- Install -----------------------------------------------------------------

install(TARGETS v4l2-test RUNTIME DESTINATION /usr/bin)


# --- Packages ----------------------------------------------------------------

# include(CPack)