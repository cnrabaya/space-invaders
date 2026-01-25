cmake_minimum_required(VERSION 3.10)
project(SpaceInvadersProject)

add_subdirectory(external/glfw)

add_definitions(-DGLEW_STATIC)
include_directories(external/glew/include)
add_library(GLEW external/glew/src/glew.c)

add_executable(SpaceInvaders main.cpp)

target_link_libraries(SpaceInvaders GLEW glfw)

if(WIN32)
    target_link_libraries(SpaceInvaders opengl32)
endif()