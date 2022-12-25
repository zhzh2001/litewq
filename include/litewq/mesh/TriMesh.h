#pragma once
#include "litewq/mesh/Mesh.h"
#include "litewq/mesh/Material.h"
#include "litewq/platform/OpenGL/GLShader.h"
#include "litewq/surface/Bezier.h"
#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace litewq {


struct Vertex {
    glm::vec3 position_;
    glm::vec3 normal_;
    glm::vec2 texture_coords_;
};


/// \brief Triangular mesh.
class TriMesh : public Mesh {
public:
    TriMesh() = delete;
    static std::unique_ptr<Mesh> from_obj(const std::string &filename);
    static std::unique_ptr<Mesh> from_bezier(const BezierSurface &bezier);
    static std::unique_ptr<Mesh> create_sphere(float radius, unsigned int n_slices, unsigned int n_stacks);
    /* A mesh may contain multiple sub-mesh and its own
     * vertex, index, uv and material, texture.
     */
    std::vector<Vertex> global_vertices_;
    std::vector<unsigned int> global_indices_;
    /* multiple submesh support. */
    struct SubMeshArea {
        std::string name_;
        /* start */
        unsigned int index_offset_;
        unsigned int index_size_;
        Material * material = nullptr;
    };
    std::vector<SubMeshArea> offsets_;

    glm::mat4 model;
    /* Assume all submesh use one shader */
    GLShader *shader = nullptr;

    /* constructors */
    TriMesh(const std::vector<Vertex> &vertex, const std::vector<unsigned int> &indices, const std::vector<SubMeshArea> &offsets)
        : global_vertices_(vertex), global_indices_(indices), offsets_(offsets) { }
    TriMesh(std::vector<Vertex> &&vertex, std::vector<unsigned int> &&indices, std::vector<SubMeshArea> &&offsets)
        : global_vertices_(vertex), global_indices_(indices), offsets_(offsets) {
    }
    TriMesh(const std::vector<Vertex> &vertex, const std::vector<unsigned int> &indices)
        : global_vertices_(vertex), global_indices_(indices) 
    {
        SubMeshArea submesh_offset;
        submesh_offset.index_offset_ = 0;
        submesh_offset.index_size_ = global_indices_.size();
        offsets_.push_back(submesh_offset);
    }
    TriMesh(std::vector<Vertex> &&vertex, std::vector<unsigned int> &&indices)
        : global_vertices_(vertex), global_indices_(indices) 
    {
        SubMeshArea submesh_offset;
        submesh_offset.index_offset_ = 0;
        submesh_offset.index_size_ = global_indices_.size();
        offsets_.push_back(submesh_offset);
    }

    virtual void render() override;
    /* portable API for debug */
    void renderSubMesh(unsigned int index);
    void setMaterialShader(unsigned int index, GLShader *shader);

    virtual void initGL() override;
    void finishGL();
private:
    bool need_rendering_ = false;
    unsigned int VAO, VBO, EBO;


};

} // end namespace litewq