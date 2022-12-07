#pragma once

#include "litewq/mesh/Mesh.h"
#include <memory>

namespace litewq {

class SkyBoxMesh : public Mesh {
public:
    /// \todo: temporary build interface like TriMesh::build_from_xxx()
    static std::unique_ptr<Mesh> build() {
        return std::make_unique<SkyBoxMesh>();
    }
    virtual void render() override;
    virtual void initGL() override;
private:
    unsigned int VAO, VBO;
};

} // end namespace litewq