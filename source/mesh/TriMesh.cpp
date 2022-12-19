#include "litewq/mesh/TriMesh.h"
#include "litewq/surface/WavefrontOBJ.h"

#include <glm/gtx/string_cast.hpp>
#include "litewq/utils/logging.h"

#include <glad/glad.h>

using namespace litewq;



std::unique_ptr<Mesh> 
TriMesh::from_obj(const std::string &filename) {
    Geometry geometry;
    GlobalVertices global_vertices;

    OBJParser parser(filename);
    parser.parse(geometry, global_vertices);

    size_t n_vertices = global_vertices.vertices.size();
    /* Change Wavefront OBJ to mesh,
     * take the average of multiple normal vector as the
     * normal of vectex. 
     */

    std::vector<Vertex> vertex(n_vertices);
    std::vector<unsigned int> indices;
    for (int i = 0; i < n_vertices; ++i) {
        vertex[i].position_ = global_vertices.vertices[i];
    }

    for (const auto &face : geometry.face_elements_) {
        for (int n_corners = 0; n_corners < face.corner_count_; ++n_corners) {
            const auto &corner = geometry.face_corners_[face.start_index_ + n_corners];
            int normal_index = corner.vertex_normal_index;
            int vertex_index = corner.vert_index;
            /* unormalized normal vector */
            vertex[vertex_index].normal_ += global_vertices.vertex_normals[normal_index];
            indices.push_back(vertex_index);
        }
    }

    auto mesh = std::make_unique<TriMesh>(std::move(vertex), std::move(indices));
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
            glm::vec3 position = glm::vec3(radius * sin(phi) * cos(theta),
                                           radius * sin(phi) * sin(theta),
                                           radius * cos(phi));
            vert.position_ = position;
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
    }

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
    glDrawElements(GL_TRIANGLE_FAN, global_indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void TriMesh::finishGL() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

