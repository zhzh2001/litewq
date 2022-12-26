
#ifndef LITEWQ_BVH_H
#define LITEWQ_BVH_H

#include "litewq/math/BoundingBox.h"
#include "litewq/mesh/TriMesh.h"

namespace litewq {

class Shape;
class Ray;

struct BVHNode {
    Bounds3 bound;
    BVHNode *left;
    BVHNode *right;
};

class BVHUtils {
public:
    BVHUtils(std::vector<Shape *> shapes, unsigned int maxShapeInNode = 1);
    BVHNode *root;

    bool intersect(const Bounds3 &bbox) const;
    bool intersect(const Ray &r) const;

    unsigned int maxShapeInNode = 1;
    std::vector<Shape *> shapes;
};

} // end namespace litewq

#endif//LITEWQ_BVH_H
