# CMake entry point
cmake_minimum_required(VERSION 3.16)
project(FinalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Find Packages ----
# Compile external dependencies 
add_subdirectory (external)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/code
    ${CMAKE_CURRENT_SOURCE_DIR}/common

	${CMAKE_CURRENT_SOURCE_DIR}/external/AntTweakBar-1.16/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-3.1.2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm-0.9.7.1
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-1.13.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp-3.0.1270/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/bullet-2.81-rev2613/src
)

# Collect all your source files
file(GLOB PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/code/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
)

# REMOVE the original controls.cpp so only controlsAlternate.cpp is used
list(REMOVE_ITEM PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/controls.cpp
)

# ---- Final executable ----
add_executable(FinalProject ${PROJECT_SOURCES})

# Copy assets to the runtime output directory (e.g., build/bin)
add_custom_command(TARGET FinalProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:FinalProject>/assets
)


# Link against the libraries provided by external/
# (check external/CMakeLists.txt if these target names differ)
target_link_libraries(FinalProject PRIVATE
    glfw
    GLEW_1130
    assimp
)

# Only FinalProject goes into build/bin
set_target_properties(FinalProject PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)