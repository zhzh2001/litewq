
#ifndef LITEWQ_GLERRORHANDLE_H
#define LITEWQ_GLERRORHANDLE_H

#include <glad/glad.h>
#include <litewq/utils/logging.h>

namespace litewq {

inline char const *glGetErrorString(GLenum const err) noexcept {
    switch (err) {
        // opengl 2 errors (8)
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "Unknown GL Error";
    }
}

/* You can define GL_NO_CHECK to disable check */
#ifndef GL_NO_CHECK
#define GL_CHECK(func) {                                    \
    func;                                                   \
    GLenum error = glGetError();                            \
    CHECK_EQ(error, GL_NO_ERROR) << glGetErrorString(error);\
}
#else
#define GL_CHECK(func) func;
#endif

#ifndef GL_NO_CHECK
#define GL_PEEK_ERROR {                                      \
    GLenum error = glGetError();                             \
    CHECK_EQ(error, GL_NO_ERROR) << glGetErrorString(error); \
}
#else
#define GL_PEEK_ERROR
#endif

} // end namespace litewq

#endif//LITEWQ_GLERRORHANDLE_H
