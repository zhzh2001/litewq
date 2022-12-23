#pragma once

#include <vector>
#include <string>

namespace litewq {



class Texture {
public:
    unsigned int texture_id_ = -1;
    unsigned int texture_unit_id_ = 0;

    Texture() = default;
    Texture(unsigned int unit_id) : texture_unit_id_(unit_id) {}
    virtual ~Texture();
    virtual void LoadTexture(const std::string &file_path);
    virtual void BindTexture() const;
};

} // end namespace litewq