cmake_minimum_required(VERSION 3.11)
project(litewq VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(GLM REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(OpenMP REQUIRED)

set(COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR} ${GLM_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR})
include_directories(${COMMON_INCLUDES})

configure_file(
    include/litewq/utils/config.h.in 
    litewq/config.h
)

# pre-build ar library.
link_directories(${CMAKE_SOURCE_DIR}/lib)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

find_package(OpenGL REQUIRED)
add_definitions(${OPENGL_DEFINITIONS})

add_subdirectory(source)
