cmake_minimum_required(VERSION 3.16)

project(opengl-samples)
set(DEPS_DIR deps)

# Open GL
find_package(OpenGL REQUIRED)
list(APPEND PROJECT_INCS ${OPENGL_INCLUDE_DIR})
list(APPEND PROJECT_LIBS ${OPENGL_LIBRARIES})

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${DEPS_DIR}/glfw)
list(APPEND PROJECT_INCS ${DEPS_DIR}/glfw/include)
list(APPEND PROJECT_LIBS glfw)

# GLEW
add_subdirectory(${DEPS_DIR}/glew)
list(APPEND PROJECT_INCS ${DEPS_DIR}/glew/include)
list(APPEND PROJECT_LIBS glew_s)

# GLM
add_subdirectory(${DEPS_DIR}/glm)
list(APPEND PROJECT_INCS ${DEPS_DIR}/glm)
list(APPEND PROJECT_LIBS glm)

# SOIL2
add_subdirectory(${DEPS_DIR}/SOIL2)
list(APPEND PROJECT_INCS ${DEPS_DIR}/SOIL2/src)
list(APPEND PROJECT_LIBS soil2)


# Target
file(GLOB_RECURSE PROJECT_SOURCES src/*)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCS})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
