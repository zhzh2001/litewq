#include "litewq/mesh/TriMesh.h"
#include "litewq/mesh/Material.h"
#include "litewq/platform/OpenGL/GLShader.h"
#include "litewq/surface/WavefrontOBJ.h"

#include <glm/gtx/string_cast.hpp>
#include "litewq/utils/logging.h"

#include <glad/glad.h>

using namespace litewq;



std::unique_ptr<Mesh> 
TriMesh::from_obj(const std::string &obj_file) {
    std::vector<std::unique_ptr<Geometry>> geometry;
    GlobalVertices global_vertices;

    OBJParser parser(obj_file);
    parser.parse(geometry, global_vertices);

    size_t n_vertices = global_vertices.vertices.size();
    /* Change Wavefront OBJ to mesh,
     * take the average of multiple normal vector as the
     * normal of vectex. 
     */

    std::vector<Vertex> vertex(n_vertices);
    std::vector<unsigned int> indices;
    std::vector<SubMeshArea> offsets;
    for (int i = 0; i < n_vertices; ++i) {
        vertex[i].position_ = global_vertices.vertices[i];
    }

    for (const auto &geom: geometry) {
        SubMeshArea submesh_offset;
        submesh_offset.name_ = geom->geometry_name_;
        submesh_offset.index_offset_ = indices.size();
        for (const auto &face : geom->face_elements_) {
            for (int n_corners = 0; n_corners < face.corner_count_; ++n_corners) {
                const auto &corner = geom->face_corners_[face.start_index_ + n_corners];
                int normal_index = corner.vertex_normal_index;
                int vertex_index = corner.vert_index;
                /* unormalized normal vector */
                vertex[vertex_index].normal_ += global_vertices.vertex_normals[normal_index];
                indices.push_back(vertex_index);
            }
        }
        submesh_offset.index_size_ = indices.size() - submesh_offset.index_offset_;
        offsets.push_back(submesh_offset);
    }

    auto mesh = std::make_unique<TriMesh>(std::move(vertex), std::move(indices), std::move(offsets));
    /* Deal with MTL*/
    std::map<std::string, std::unique_ptr<MTLMaterial>> materials;
    for (const auto& mtl_library : parser.get_mtl_libraries()) {
        MTLParser mtl_parser(mtl_library, obj_file);
        mtl_parser.parse(materials);
    }

    for (unsigned int i = 0; i < geometry.size(); ++i) {
        const auto &geom = geometry[i];
        for (const auto &mat_name : geom->material_order_) {
            auto &mtl = materials.at(mat_name);
            auto *phong_mat = PhongMaterial::Create(mtl.get(), nullptr);
            mesh->material_map_[i] = (Material *)phong_mat;
        }
    }
    return mesh;
}

std::unique_ptr<Mesh> 
TriMesh::from_bezier(const BezierSurface &bezier) {

    std::vector<Vertex> vertex;
    std::vector<unsigned int> indices;

    unsigned int u_mesh = bezier.n_us * 3;
    unsigned int v_mesh = bezier.n_vs * 3;
    float du = 1.0 / (u_mesh - 1);
    float dv = 1.0 / (v_mesh - 1);

    // vertex.resize(u_mesh * v_mesh);
    // indices.resize(u_mesh * v_mesh * 2 * 3);
    Vertex vert;
    for (int i = 0; i < u_mesh; ++i) {
        float u = du * i;
        for (int j = 0; j < v_mesh; ++j) {
            float v = dv * j;
            glm::vec3 position = bezier.getPoint(u, v);
            vert.position_ = position;
            vertex.push_back(vert);
        }
    } 

    for (int i = 0; i < u_mesh - 1; ++i) {
        for (int j = 0; j < v_mesh - 1; ++j) {
            unsigned int dudv_index = i * v_mesh + j;
            indices.push_back(dudv_index);
            indices.push_back(dudv_index + 1);
            indices.push_back(dudv_index + v_mesh);

            indices.push_back(dudv_index + 1);
            indices.push_back(dudv_index + v_mesh);
            indices.push_back(dudv_index + v_mesh + 1);
        }
    }


    auto mesh = std::make_unique<TriMesh>(std::move(vertex), std::move(indices));
    return mesh;
}

std::unique_ptr<Mesh>
TriMesh::create_sphere(float radius, unsigned int n_slices, unsigned int n_stacks)
{
    std::vector<Vertex> vertex;
    std::vector<unsigned int> indices;

    float dphi = glm::pi<float>() / n_stacks;
    float dtheta = 2 * glm::pi<float>() / n_slices;

    Vertex vert;
    for (int i = 0; i < n_stacks; ++i)
    {
        float phi = dphi * i;
        for (int j = 0; j < n_slices; ++j)
        {
            float theta = dtheta * j;
            vert.position_ = glm::vec3(radius * sin(phi) * cos(theta),
                                           radius * sin(phi) * sin(theta),
                                           radius * cos(phi));
            /* respect to origin */
            vert.normal_ = vert.position_;
            vertex.push_back(vert);
        }
    }

    for (int i = 0; i < n_stacks - 1; ++i)
    {
        for (int j = 0; j < n_slices - 1; ++j)
        {
            unsigned int dudv_index = i * n_slices + j;
            indices.push_back(dudv_index);
            indices.push_back(dudv_index + 1);
            indices.push_back(dudv_index + n_slices);

            indices.push_back(dudv_index + 1);
            indices.push_back(dudv_index + n_slices);
            indices.push_back(dudv_index + n_slices + 1);
        }
        /* Deal with edge triangles*/
        unsigned int dudv_index = i * n_slices + n_slices - 1;
        indices.push_back(dudv_index);
        indices.push_back(dudv_index + 1 - n_slices);
        indices.push_back(dudv_index + n_slices);

        indices.push_back(dudv_index + 1 - n_slices);
        indices.push_back(dudv_index + n_slices);
        indices.push_back(dudv_index + 1);
    }

    CHECK_EQ(indices.size(), (n_stacks - 1) * n_slices * 6) << "Wrong sphere triangle splits";
    auto mesh = std::make_unique<TriMesh>(std::move(vertex), std::move(indices));
    return mesh;
}

void TriMesh::initGL() {
    LOG(INFO) << "TriMesh Init: ";
    LOG(INFO) << "Totol Vertex: " << global_vertices_.size();
    // for (const auto &vert : global_vertices_) {
    //     std::cout << glm::to_string(vert.position_) << std::endl;
    // }
    LOG(INFO) << "Total Indices:" << global_indices_.size(); 

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); 
    /* copy vertex data */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, global_vertices_.size() * sizeof(Vertex), 
                global_vertices_.data(), GL_STATIC_DRAW);
    /* copy index data */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, global_indices_.size() * sizeof(unsigned int),
                global_indices_.data(), GL_STATIC_DRAW);

    /* set vertex position data */
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    /* set vertex normal data */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal_));
    /* set vertex uv coordinate */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texture_coords_));

    /* Unbind VAO */
    glBindVertexArray(0);
}

void TriMesh::render() {
    glBindVertexArray(VAO);
    for (unsigned int i = 0; i < offsets_.size(); ++i) {
        /* if corresponding submesh has material */
        if (material_map_.count(i)) {
            material_map_.at(i)->updateMaterial();
        }
        auto &submesh = offsets_[i];
        glDrawElements(GL_TRIANGLES, submesh.index_size_,
                       GL_UNSIGNED_INT, (void *)(submesh.index_offset_ * sizeof(GLuint)));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void TriMesh::finishGL() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}



void TriMesh::renderSubMesh(unsigned int index) {
    glBindVertexArray(VAO);
    /* if corresponding submesh has material, and bind shader */
    if (material_map_.count(index)) {
        material_map_[index]->updateMaterial();
    }
    auto &submesh = offsets_[index];
    glDrawElements(GL_TRIANGLES, submesh.index_size_,
                   GL_UNSIGNED_INT, (void *)(submesh.index_offset_ * sizeof(GLuint)));
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

}

void TriMesh::setMaterialShader(unsigned int index, GLShader *shader) {
    auto &submesh = offsets_[index];
    LOG(INFO) << "Set submesh index: " << index << " Shader: " << "\n";
    material_map_.at(index)->shader = shader;
}
