#include "litewq/math/BVH.h"
#include "litewq/math/Shape.h"

#include <algorithm>

using namespace litewq;


static BVHNode *BuildBVH(std::vector<Shape *> shapes) {
    BVHNode *Node = new BVHNode;

    Bounds3 NodeBound;
    for (auto *shape : shapes) {
        NodeBound = Union(NodeBound, shape->WorldBound());
    }
    if (shapes.size() == 1) {
        Node->bound = shapes[0]->WorldBound();
        Node->left = nullptr;
        Node->right = nullptr;
        return Node;
    }
    else if (shapes.size() == 2) {
        Node->left = BuildBVH(std::vector{shapes[0]});
        Node->right = BuildBVH(std::vector{shapes[1]});

        Node->bound = Union(Node->left->bound, Node->right->bound);
        return Node;
    }
    else {
        Bounds3 centroidBounds;
        for (auto *shape : shapes) {
            centroidBounds =
                    Union(centroidBounds, shape->WorldBound());
        }
        /* Estimate SAH */
        int dim = centroidBounds.maxExtent();
        switch (dim) {
            case 0:
                std::sort(shapes.begin(), shapes.end(), [](auto f1, auto f2) {
                    return f1->WorldBound().Centroid().x <
                           f2->WorldBound().Centroid().x;
                });
                break;
            case 1:
                std::sort(shapes.begin(), shapes.end(), [](auto f1, auto f2) {
                    return f1->WorldBound().Centroid().y <
                           f2->WorldBound().Centroid().y;
                });
                break;
            case 2:
                std::sort(shapes.begin(), shapes.end(), [](auto f1, auto f2) {
                    return f1->WorldBound().Centroid().z <
                           f2->WorldBound().Centroid().z;
                });
                break;
        }

        auto begin = shapes.begin();
        auto mid = shapes.begin() + (shapes.size() / 2);
        auto end = shapes.end();

        auto LeftShapes = std::vector<Shape *>(begin, mid);
        auto RightShapes = std::vector<Shape *>(mid, end);


        Node->left = BuildBVH(LeftShapes);
        Node->right = BuildBVH(RightShapes);

        Node->bound = Union(Node->left->bound, Node->right->bound);
    }

    return Node;
}

BVHUtils::BVHUtils(std::vector<Shape *> shapes, unsigned int maxShapeInNode) :
    maxShapeInNode(maxShapeInNode), shapes(std::move(shapes))
{

}