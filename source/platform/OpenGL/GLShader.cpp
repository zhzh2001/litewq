#include "litewq/platform/OpenGL/GLShader.h"
#include "litewq/platform/OpenGL/GLErrorHandle.h"

#include <glm/gtc/type_ptr.hpp>
#include "glad/glad.h"


#include <iostream>

using namespace litewq;

static GLShader *current = nullptr;

GLShader *GLShader::GetCurrentShader() {
    return current;
}

GLShader::~GLShader() {
    glDeleteProgram(render_id_);
}

GLShader::GLShader(const std::string &vertex_src, const std::string &frag_src) {
        GLint success;
        GLchar infoLog[1024];
        const char *vertex_code = vertex_src.c_str();
        const char *frag_code = frag_src.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_code, NULL);
        glCompileShader(vertex);

        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: vertex " << "\n" << infoLog << "\n";
        }
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &frag_code, NULL);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: fragment " << "\n" << infoLog << "\n";
        }
        // shader Program

        render_id_ = glCreateProgram();
        glAttachShader(render_id_, vertex);
        glAttachShader(render_id_, fragment);
        glLinkProgram(render_id_);

        glGetProgramiv(render_id_, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(render_id_, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: link program" << "\n" << infoLog << "\n";
        }
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
}

void GLShader::Bind() const {
    current = const_cast<GLShader *>(this);
    GL_CHECK(glUseProgram(render_id_));
}

void GLShader::UnBind() const {
    current = nullptr;
    GL_CHECK(glUseProgram(render_id_));
}

void GLShader::updateUniformInt(const std::string &name, const int value) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniform1i(location, value));
}

void GLShader::updateUniformFloat(const std::string &name, const float value) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniform1f(location, value));
}

void GLShader::updateUniformFloat3(const std::string &name, const glm::vec3 &vec) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniform3f(location, vec.x, vec.y, vec.z));
}

void GLShader::updateUniformFloat3v(const std::string &name, unsigned count, const float *value) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniform3fv(location, count, value));
}

void GLShader::updateUniformFloat4(const std::string &name, const glm::vec4 &vec) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniform4f(location, vec.x, vec.y, vec.z, vec.w));
}

void GLShader::updateUniformMat3(const std::string &name, const glm::mat3 &mat) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat)));
}

void GLShader::updateUniformMat4(const std::string &name, const glm::mat4 &mat) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat)));
}

void GLShader::compile(const char *source) {

}

void GLShader::createProgram() {

}