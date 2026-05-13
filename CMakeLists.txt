cmake_minimum_required(VERSION 3.5.0)
option (ENABLE_TESTS "Enable tests" OFF)
option (ENABLE_GUI "Enable opencv gui (legacy)" OFF)

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
find_package(Threads REQUIRED)

# OpenCV 4.5.0 or newer/greater
find_package(OpenCV REQUIRED)
if(OpenCV_VERSION VERSION_LESS "4.5.0")
    message(FATAL_ERROR "OpenCV version must be at least 4.5.0")
endif()

if(OPENCV_FOUND AND ENABLE_GUI)
        add_compile_definitions(WITH_GUI)
endif()

if (OPENCV_FOUND)
        # add_compile_definitions(WITH_CCM)
        include_directories(${OpenCV_INCLUDE_DIRS})
endif()



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
        src/cv/imageConvertCPU.cpp
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
find_package(PkgConfig REQUIRED)

pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
pkg_search_module(GSTREAMER_SDP REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
pkg_search_module(GSTREAMER_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)
pkg_search_module(GSTREAMER_VIDEO REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.4)

pkg_search_module(LIBAV_CODEC  REQUIRED libavcodec)
pkg_search_module(LIBAV_FORMAT REQUIRED libavformat)
pkg_search_module(LIBAV_UTIL   REQUIRED libavutil)
pkg_search_module(LIBAV_SWS    REQUIRED libswscale)

# Library sources
add_library(sinks
        STATIC
        src/sinks/gstreamerSink.cpp
        src/sinks/imagesaversink.cpp
        src/sinks/videoSink.cpp
       )
target_include_directories(sinks PUBLIC include
        ${GSTREAMER_INCLUDE_DIRS} ${GSTREAMER_APP_INCLUDE_DIRS}
        ${LIBAV_CODEC_INCLUDE_DIRS} ${LIBAV_FORMAT_INCLUDE_DIRS}
        ${LIBAV_UTIL_INCLUDE_DIRS} ${LIBAV_SWS_INCLUDE_DIRS})
target_link_libraries(sinks PRIVATE cv utils Threads::Threads
        ${GSTREAMER_APP_LIBRARIES} ${GSTREAMER_LIBRARIES}
        ${LIBAV_CODEC_LIBRARIES} ${LIBAV_FORMAT_LIBRARIES}
        ${LIBAV_UTIL_LIBRARIES} ${LIBAV_SWS_LIBRARIES})
if (OPENCV_FOUND)
        target_link_libraries(sinks PUBLIC ${OpenCV_LIBS})
endif()
        # Library gui
add_library(gui
        STATIC
        src/gui/framebuffer.cpp)
target_include_directories(gui PUBLIC include)
target_link_libraries(gui PRIVATE cv sources)
if (OPENCV_FOUND AND ENABLE_GUI)
        target_sources(gui PRIVATE 
                src/gui/opencv/viewer.cpp
                src/gui/opencv/convert.cpp
                src/gui/opencv/draw.cpp
                src/gui/opencv/text.cpp
                src/gui/opencv/mouseevent.cpp
                src/gui/opencv/control.cpp
                src/gui/opencv/button.cpp
                src/gui/opencv/window.cpp
                src/gui/opencv/mainwindow.cpp
                src/gui/opencv/ccmwindow.cpp)
        target_link_libraries(gui PUBLIC ${OpenCV_LIBS})
endif()

# Library runner
add_library(runner
        STATIC
        src/runners/basicstreamrunner.cpp
        src/runners/imagesaverrunner.cpp
        src/runners/videoRunner.cpp
        src/runners/streamrunner.cpp
        src/runners/isprunner.cpp
        src/runners/noisetestrunner.cpp
        src/runners/gstreamerRunner.cpp
        src/runners/socketclientrunner.cpp)
target_include_directories(runner PUBLIC include ${GSTREAMER_INCLUDE_DIRS} ${GSTREAMER_APP_INCLUDE_DIRS} )
target_link_libraries(runner PRIVATE cv utils gui network sources sinks)


# --- Executables -------------------------------------------------------------

# Executable v4l2-test
add_executable(v4l2-test 
        src/v4l2-test/main.cpp)
target_include_directories(v4l2-test PRIVATE ${PROJECT_BINARY_DIR}/include)
target_link_libraries(v4l2-test PRIVATE cv utils gui network sources runner)

# add_custom_command(
#         TARGET v4l2-test POST_BUILD 
#         COMMAND scp ARGS v4l2-test root@imx8mp-var-dart:/home/root
#         VERBATIM)

# --- Tests ------------------------------------------------------------------

if (ENABLE_TESTS)
      
# Add GTest package
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(v4l2_tests
    tests/utils_test.cpp
    tests/cv_test.cpp
    tests/imageConvertCPU_test.cpp
)
target_link_libraries(v4l2_tests
    GTest::gtest_main
    utils
    cv
    ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(v4l2_tests)

endif()

# --- Install -----------------------------------------------------------------

install(TARGETS v4l2-test RUNTIME DESTINATION /usr/bin)


# --- Packages ----------------------------------------------------------------

# include(CPack)