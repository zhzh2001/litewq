
#ifndef LITEWQ_SHAPE_H
#define LITEWQ_SHAPE_H

#include <glm/glm.hpp>

namespace litewq {

class TriMesh;
class Bounds3;

class Shape {
public:
    virtual Bounds3 ObjectBound() = 0;
    virtual Bounds3 WorldBound();

    glm::mat4 *ObjectToWorld;
};

class Triangle : public  Shape {
public:

    virtual Bounds3 ObjectBound() override;
    TriMesh *mesh;
    const int *v;
};

} // end namespace litewq


#endif//LITEWQ_SHAPE_H
