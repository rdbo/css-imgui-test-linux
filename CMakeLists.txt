cmake_minimum_required(VERSION 3.21.3)
project(test)

set(CMAKE_CXX_STANDARD 11)
find_package(SDL2 REQUIRED)

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
file(GLOB_RECURSE IMGUI_SRC ${IMGUI_DIR}/*.cpp)

set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)
file(GLOB_RECURSE TEST_SRC ${TEST_DIR}/*.cpp)

add_library(test SHARED ${TEST_SRC} ${IMGUI_SRC})
target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR} ${TEST_DIR} ${IMGUI_DIR} ${SDL2_INCLUDE_DIRS})
target_compile_options(test PUBLIC -g -ggdb -m32 -shared -fPIC -fpermissive -pthread)
target_link_options(test PUBLIC -m32 -static-libstdc++ -pthread LINKER:--no-undefined,--no-allow-shlib-undefined)
target_link_libraries(test PUBLIC stdc++ m dl GL SDL2)
