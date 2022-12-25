set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB LIBSRC ./*/*.cpp)
file(GLOB OPENGL_SRC ./platform/OpenGL/*)

# add_library(${CMAKE_PROJECT_NAME} ${LIBSRC} ${OPENGL_SRC})
add_executable(${PROJECT_NAME} ${LIBSRC} ${OPENGL_SRC} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW3_LIBRARY} OpenMP::OpenMP_CXX OpenGL::GL)


if (ASAN)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fsanitize=address")
    target_link_options(${PROJECT_NAME} PRIVATE "-fsanitize=address")
endif()