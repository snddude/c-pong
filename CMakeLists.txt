cmake_minimum_required(VERSION 4.3)
project(pong)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

set(SDL_GPU OFF)
set(SDL_TRAY OFF)
set(SDL_POWER OFF)
set(SDL_STATIC ON)
set(SDL_CAMERA OFF)
set(SDL_DIALOG OFF)
set(SDL_HAPTIC OFF)
set(SDL_HIDAPI OFF)
set(SDL_SENSOR OFF)
set(SDL_SHARED OFF)
set(SDL_JOYSTICK OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_NOTIFICATION OFF)

add_subdirectory(vendor/SDL)

file(GLOB_RECURSE SRCS ./src/*.c)
file(COPY res DESTINATION ${CMAKE_BINARY_DIR}/dist)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ./include/)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static)
set_target_properties(${PROJECT_NAME} PROPERTIES 
    COMPILE_FLAGS "-Wall -Wextra -pedantic -Wno-unused-parameter -g0 -O3"
    LINK_FLAGS    "-s"
)
