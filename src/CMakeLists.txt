cmake_minimum_required(VERSION 3.10)
project(xgp-v3-screen)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ui)

file(GLOB_RECURSE UI_SOURCES "ui/*.c")
add_executable(zz_xgp_screen main.c ${UI_SOURCES})
target_link_libraries(zz_xgp_screen lvgl lvgl::examples lvgl::demos lvgl::thorvg m pthread)

install(TARGETS zz_xgp_screen DESTINATION bin)
