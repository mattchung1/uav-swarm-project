# CMake entry point
cmake_minimum_required(VERSION 3.16)
project(FinalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put built executable(s) under build/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Find Packages ----
# Find OpenGL
find_package(OpenGL REQUIRED)
# Find GLFW
find_package(glfw3 REQUIRED)
# Find GLEW (if you use it)
find_package(GLEW REQUIRED)

# ---- Source files ----
# Grab all .cpp files from code/ and common/
file(GLOB PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/code/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
)

add_executable(FinalProject ${SOURCES})

target_include_directories(FinalProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/code
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# Link libraries
target_link_libraries(FinalProject PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
)