cmake_minimum_required(VERSION 3.19)
project(kingdom)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)

# Source Files
file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Libraries
find_library(SDL2_LIB SDL2 PATHS "../../../lib/SDL2/lib/x64")

# Include directories
include_directories("lib/SDL2/include") # SDL2 headers
include_directories("lib/glm/include")  # Math headers
include_directories("lib/glad/include") # OpenGL bindings
include_directories("lib/spdlog/include") # Logging library
include_directories("lib/entt/single_include") # ECS System
include_directories("lib/toml11/include") # TOML file reading

add_executable(kingdom ${SOURCE_FILES})
target_link_libraries(kingdom ${SDL2_LIB})

# Copy content directory to build path
add_custom_command(TARGET kingdom POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/content/ $<TARGET_FILE_DIR:kingdom>/content/)

# Copy SDL2.dll to build path (Windows only)
add_custom_command(TARGET kingdom POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "../../../../lib/SDL2/lib/x64/SDL2.dll" $<TARGET_FILE_DIR:kingdom>)