#include "litewq/platform/OpenGL/GLShader.h"

#include <glm/gtc/type_ptr.hpp>
#include "glad/glad.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace litewq;

GLShader::~GLShader() {
    glDeleteProgram(render_id_);
}

GLShader::GLShader(const std::string &vertex_src, const std::string &frag_src) {
        GLint success;
        GLchar infoLog[1024];
        std::ifstream vertex_file(vertex_src);
        std::ifstream frag_file(frag_src);
        if (!vertex_file.is_open() || !frag_file.is_open())
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            return;
        }
        std::stringstream vertex_stream, frag_stream;
        vertex_stream << vertex_file.rdbuf();
        frag_stream << frag_file.rdbuf();
        vertex_file.close();
        frag_file.close();
        std::string vertex_code = vertex_stream.str();
        std::string frag_code = frag_stream.str();
        char *vertex_code_c = const_cast<char *>(vertex_code.c_str());
        char *frag_code_c = const_cast<char *>(frag_code.c_str());

        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_code_c, NULL);
        glCompileShader(vertex);

        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: vertex " << "\n" << infoLog << "\n";
        }
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &frag_code_c, NULL);
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
    glUseProgram(render_id_);
}

void GLShader::UnBind() const {
    glUseProgram(render_id_);
}

void GLShader::updateUniformFloat3(const std::string &name, const glm::vec3 &vec) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
glUniform3f(location, vec.x, vec.y, vec.z);
}

void GLShader::updateUniformFloat4(const std::string &name, const glm::vec4 &vec) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

void GLShader::updateUniformMat3(const std::string &name, const glm::mat3 &mat) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void GLShader::updateUniformMat4(const std::string &name, const glm::mat4 &mat) {
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void GLShader::updateUniformInt(const std::string &name, int value)
{
    GLint location = glGetUniformLocation(render_id_, name.c_str());
    glUniform1i(location, value);
}

void GLShader::compile(const char *source) {

}

void GLShader::createProgram() {

}
