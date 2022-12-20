#pragma once

#include <vector>
#include <string>

namespace litewq {

class Texture {
public:
    unsigned int texture_unit_id_ = 0;
    unsigned int texture_id_ = -1;

    virtual ~Texture();
    virtual void LoadTexture(const std::string &file_path);
    virtual void BindTexture() const;
};

} // end namespace litewq