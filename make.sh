#!/bin/bash

SCRIPT_DIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
RELEASE_DIR=$SCRIPT_DIR/release

usage() {
	echo "Usage: $0 [options]                                        "
	echo "                                                           "
	echo "Supported options:                                         "
    echo "-a, --all                 Build nativ and aarch64 cross    "
    echo "-c, --cross               Build aarch64 cross              "
}

build()
{
    BUILD_DIR=${SCRIPT_DIR}/build_$1
    rm -Rf ${BUILD_DIR}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../toolchain/$2.cmake ..
    cmake --build .
    mkdir -p ${RELEASE_DIR}
    tar -zcvf ${RELEASE_DIR}/v4l2-test-0.3.0-linux-$1.tar.gz v4l2-test
}

build_generic()
{
    build generic gnu-generic-toolchain
}

build_x86-64()
{
    build x86-64 gnu-generic-toolchain
}

build_arm64()
{
    build arm64 aarch64-cross-toolchain
}

while [ $# != 0 ] ; do
	option="$1"
	shift

	case "${option}" in
        -a|--all)
            build_x86-64
            build_arm64
            exit 0
            ;;
        -c|--cross)
            build_arm64
            exit 0
            ;;
        -h|--help)
            usage
            exit 0
		    ;;
        *)
            echo "Unknown option ${option}"
            exit 1
            ;;
        esac
done

build_generic