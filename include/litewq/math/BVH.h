
#ifndef LITEWQ_BVH_H
#define LITEWQ_BVH_H

#include "litewq/math/BoundingBox.h"
#include <vector>


namespace litewq {

class Shape;
class Ray;

struct BVHNode {
    Bounds3 bound;
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;
};

class BVHUtils {
public:
    BVHUtils(std::vector<Shape *> &shapes, unsigned int maxShapeInNode = 1);
    BVHNode *root = nullptr;

    bool intersect(const Bounds3 &bbox) const {
        return root != nullptr && intersect(root, bbox);
    }
    bool intersect(BVHNode *node, const Bounds3 &bbox) const;
    Bounds3 WorldBound() const {
        return root->bound;
    }

    unsigned int maxShapeInNode = 1;
    std::vector<Shape *> shapes;
};

} // end namespace litewq

#endif//LITEWQ_BVH_H
