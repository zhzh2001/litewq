#pragma once

#include <glm/glm.hpp>

#include <string>
#include <cstddef>

namespace litewq {

class GLShader {
public:
    GLShader(const std::string &vertex_src, const std::string &frag_src);
    ~GLShader();

    void Bind() const;
    void UnBind() const;

    void updateUniformFloat3(const std::string &name, const glm::vec3 &vec);
    void updateUniformFloat4(const std::string &name, const glm::vec4 &vec);
    void updateUniformMat3(const std::string &name, const glm::mat3 &mat);
    void updateUniformMat4(const std::string &name, const glm::mat4 &mat);
private:
    void compile(const char *source);
    void createProgram();
    uint32_t render_id_;
};

} // end namespace litewq
