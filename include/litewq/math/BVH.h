
#ifndef LITEWQ_BVH_H
#define LITEWQ_BVH_H

#include "litewq/math/BoundingBox.h"
#include "litewq/mesh/TriMesh.h"

namespace litewq {

class Shape;

struct BVHNode {
    Bounds3 bound;
    BVHNode *left;
    BVHNode *right;
};

class BVHUtils {
public:
    BVHUtils(std::vector<Shape *> shapes, unsigned int maxShapeInNode = 1);
    BVHNode *root;


    unsigned int maxShapeInNode = 1;
    std::vector<Shape *> shapes;
};

} // end namespace litewq

#endif//LITEWQ_BVH_H
