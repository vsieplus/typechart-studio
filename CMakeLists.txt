cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # show warnings    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g3")

endif()

project(typechart-studio VERSION 1.0.0)
configure_file(src/versionfile.h.in versionconfig.h)

message(STATUS "Project name: ${PROJECT_NAME}")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")

add_subdirectory(src)
add_subdirectory(include)

set(BUILD_DIRS
    "build/linux/Debug"
    "build/linux/Release"
    "build/windows32/Debug"
    "build/windows32/Release"
    "build/windows64/Debug"
    "build/windows64/Release")

# SDL2 > 2.0.17 to work with imgui
find_package(SDL2 2.0.17 REQUIRED)
find_package(SDL2_image 2.6.3 REQUIRED)
find_package(OpenAL 1.21.0 REQUIRED)

set(SNDFILE_LIBRARIES "sndfile")
set(SDL2_IMAGE_LIBRARIES "SDL2_image")

# IMGUI
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(IMGUI_FD_DIR ${EXTERNAL_DIR}/ImGuiFileDialog)
set(IMGUIZMO_DIR ${EXTERNAL_DIR}/ImGuizmo)
set(ICNFNTCPP_DIR ${EXTERNAL_DIR}/IconFontCppHeaders)
set(JSON_DIR ${EXTERNAL_DIR}/nlohmann)

target_sources(${PROJECT_NAME}
PRIVATE
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer.cpp
PRIVATE
    ${IMGUI_FD_DIR}/ImGuiFileDialog.cpp
PRIVATE
    ${IMGUIZMO_DIR}/ImSequencer.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${SDL2_INCLUDE_DIRS}
    PUBLIC ${IMGUI_DIR}
    PUBLIC ${IMGUI_DIR}/backends
    PUBLIC ${IMGUI_FD_DIR}
    PUBLIC ${IMGUIZMO_DIR}
    PUBLIC ${ICNFNTCPP_DIR}
    PUBLIC ${JSON_DIR}
    PUBLIC ${OPENAL_INCLUDE_DIR}
    PUBLIC ${BUILD_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${SNDFILE_LIBRARIES}
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release" AND ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "Linking libgcc, libstdc++ statically for mingw")
    target_link_libraries(${PROJECT_NAME} -static-libgcc -static-libstdc++ -lpthread)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # macOS ...
endif()

message(STATUS "Using CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "Using CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
