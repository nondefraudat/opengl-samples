cmake_minimum_required(VERSION 3.14)

project(opengl-samples)

file(GLOB_RECURSE PROJECT_SOURCES src/*)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
