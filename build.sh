#!/bin/bash
# Usage:
#   bash build.sh [build-system] [build-type]
#   [build-system] : {windows32, windows64, mac, (linux -- default)}
#   [build-type] : (Release, (Debug -- default)} - **CASE SENSITIVE**

ROOT_DIR=$(pwd)
PRG_NAME="typechart-studio"

BUILD_SYSTEM="linux"
BUILD_TYPE="Debug"

if [[ -z "$1" ]]; then
    echo "Using default build system: ${BUILD_SYSTEM}"
else
    BUILD_SYSTEM="$1"
fi

if [[ -z "$2" ]]; then
    echo "Using default build type: ${BUILD_TYPE}"
else
    BUILD_TYPE="$2"
fi

BUILD_DIR="build/${BUILD_SYSTEM}/${BUILD_TYPE}"

# clean existing build dir for release builds
if [[ "${BUILD_TYPE}" == "Release" ]]; then
    rm -rf "${BUILD_DIR}"
fi

if [[ "${BUILD_SYSTEM}" == "windows32" ]]; then
    PRG_FILE="${PRG_NAME}.exe"

    echo "Building for windows 32-bit"
    cmake -DCMAKE_TOOLCHAIN_FILE=/home/ryan/Documents/dev/cmake/toolchain-mingw32-x86.cmake \
          -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -S . -B "${BUILD_DIR}"
elif [[ "${BUILD_SYSTEM}" == "windows64" ]]; then
    PRG_FILE="${PRG_NAME}.exe"

    echo "Building for windows 64-bit"
    cmake -DCMAKE_TOOLCHAIN_FILE=/home/ryan/Documents/dev/cmake/toolchain-mingw32-x64.cmake \
          -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -S . -B "${BUILD_DIR}"
elif [[ "${BUILD_SYSTEM}" == "mac" ]]; then
    PRG_FILE="${PRG_NAME}"
    echo "Building for mac [not yet impelmented]"
elif [[ "${BUILD_SYSTEM}" == "linux" ]]; then
    PRG_FILE="${PRG_NAME}"

    echo "Building for Linux [default]"
    cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
          -S . -B "${BUILD_DIR}"
else
    echo "Build for system ${BUILD_SYSTEM} not yet supported, exiting..."
    exit 1
fi

echo "-- BUILD_DIR: ${BUILD_DIR}"
cd "${BUILD_DIR}"
make || exit 1

mv src/${PRG_FILE} .

if [[ "${BUILD_TYPE}" == "Release" ]]; then
    cd "${ROOT_DIR}"

    VERSION=0.1.1 # this should match what is in the CMakeLists.txt file
    OUT_DIR="releases/${VERSION}/${BUILD_SYSTEM}"

    mkdir -p "${OUT_DIR}"
    echo "Creating release under ${OUT_DIR}"

    # copy README
    cp README.txt "${OUT_DIR}"

    # copy the executable and version config
    cp "${BUILD_DIR}/${PRG_FILE}" "${OUT_DIR}"

    # for linux copy the 'launcher script' so .so files can be found
    if [[ "${BUILD_SYSTEM}" == "linux" ]]; then
        cp "typechart-studio.sh" "${OUT_DIR}"
    fi

    echo "-- Copying resources"

    # copy game resources and notices
    cp -r license "${OUT_DIR}"
    cp -r fonts "${OUT_DIR}"
    cp -r sounds "${OUT_DIR}"

    echo "-- Copying libraries"
    # copy appropriate dynamic libraries
    libs=("libsndfile" "libogg" "libvorbis" "libopus" "libFLAC")
    winlibs=("SDL2" "SDL2_image" "OpenAL32" "zlib1" "libmpg123-0" "libmp3lame-0")
    if [[ "${BUILD_SYSTEM}" == "linux" ]]; then
        LIB_DIR="/usr/local/lib"
        LIB_EXT="so"

        libs+=("libSDL2" "libSDL2_image" "libmpg123" "libmp3lame")
    elif [[ "${BUILD_SYSTEM}" == "windows32" ]]; then
        LIB_DIR="/usr/i686-w64-mingw32/bin"
        LIB_EXT="dll"

        libs+=( "${winlibs[@]}" )
        libs+=( "libgcc_s_sjlj-1" "libgcc_s_dw2-1" "libssp-0" "libwinpthread-1")
    elif [[ "${BUILD_SYSTEM}" == "windows64" ]]; then
        LIB_DIR="/usr/x86_64-w64-mingw32/bin"
        LIB_EXT="dll"

        libs+=( "${winlibs[@]}" )
        libs+=( "libssp-0" )
    elif [[ "${BUILD_SYSTEM}" == "mac" ]]; then
        LIB_DIR="/usr/local/lib"
        LIB_EXT="dylib"
    fi

    for lib in ${libs[@]}; do
        libpath="${LIB_DIR}/${lib}.${LIB_EXT}"

        echo "-- Copying ${libpath}"
        liboutname=$(basename $(readlink -f ${libpath}))
        cp -L "${libpath}" "${OUT_DIR}/${liboutname}"
    done

    # rename libogg.dll to ogg.dll for windows builds
    # copy mingw libs
    if [[ "${BUILD_SYSTEM}" == "windows32" || "${BUILD_SYSTEM}" == "windows64" ]]; then
        mv "${OUT_DIR}/libogg.dll" "${OUT_DIR}/ogg.dll"
    fi

    # linux specific
    linuxlibs=( "libopenal.so.1" "libsndio.so.7.0" )
    if [[ "${BUILD_SYSTEM}" == "linux" ]]; then
        for linuxlib in ${linuxlibs[@]}; do
            cp -L "/usr/lib/x86_64-linux-gnu/${linuxlib}" "${OUT_DIR}"
        done        

        mv "${OUT_DIR}/libSDL2-2.0.so.0.18.2" "${OUT_DIR}/libSDL2-2.0.so.0"
        mv "${OUT_DIR}/libSDL2_image-2.0.so.0.2.3" "${OUT_DIR}/libSDL2_image-2.0.so.0"
    fi

    # create a zip
    cd "releases/${VERSION}"
    ZIP_OUT="${PRG_NAME}-${VERSION}-${BUILD_SYSTEM}.zip"

    # delete previous zip if already exists
    rm -f ${ZIP_OUT}
    mv "${BUILD_SYSTEM}" "${PRG_NAME}-${VERSION}-${BUILD_SYSTEM}"
    zip -r "${ZIP_OUT}" "${PRG_NAME}-${VERSION}-${BUILD_SYSTEM}"

    rm -rf "${PRG_NAME}-${VERSION}-${BUILD_SYSTEM}"

    echo "Zip created at ${ZIP_OUT}"
fi