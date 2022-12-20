#include "litewq/mesh/Texture.h"

#include "litewq/utils/Loader.h"
#include "litewq/utils/logging.h"

#include "glad/glad.h"
#include "stb/stb_image.h"
using namespace litewq;


constexpr GLuint DEFAULT_TEXTURE_WRAP = GL_REPEAT;
constexpr GLuint DEFAULT_TEXTURE_FILTER = GL_LINEAR;


Texture::~Texture() {
    if (texture_id_ != -1) {
        glDeleteTextures(1, &texture_id_);
    }
}

void Texture::LoadTexture(const std::string &file_path) {
    if (texture_id_ != -1) {
        LOG(WARNING) << "Already loaded texture ID: " << texture_id_;
        glDeleteTextures(1, &texture_id_);
    }

    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load(
        file_path.c_str(),
        &width, &height, &channels, 0
    );
    CHECK(data) << "Failed to load texture: " << file_path;
    GLenum format;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, DEFAULT_TEXTURE_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, DEFAULT_TEXTURE_FILTER);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    /// \attention: auto generate mipmap 
    /// as we set mipmap level in glTexImage2D is zero (basic level).
    glGenerateMipmap(GL_TEXTURE_2D);
    LOG(INFO) << "ID: " << texture_id_ << " Unit: " << texture_unit_id_
            << " Load texture: " << file_path 
            << " height: " << height << " width: " << width;

    stbi_image_free(data);
}

void Texture::BindTexture() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    // LOG(INFO) << "Bind texture ID: " <<  texture_id_;
}