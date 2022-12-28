
#ifndef LITEWQ_SHAPE_H
#define LITEWQ_SHAPE_H

#include <glm/glm.hpp>

namespace litewq {

class TriMesh;
class Bounds3;

class Shape {
public:
    Shape() = delete;
    explicit Shape(glm::mat4 *ObjectToWorld) :
            ObjectToWorld(ObjectToWorld) {}
    virtual Bounds3 ObjectBound() = 0;
    virtual Bounds3 WorldBound();

    glm::mat4 *ObjectToWorld;
};

class Triangle : public  Shape {
public:
    Triangle() = delete;
    Triangle(glm::mat4 *ObjectToWorld, TriMesh *Mesh, unsigned int *V) :
            Shape(ObjectToWorld), mesh(Mesh), v(V) {}
    virtual Bounds3 ObjectBound() override;
    TriMesh *mesh;
    const unsigned int *v;
};

} // end namespace litewq


#endif//LITEWQ_SHAPE_H
