#include "litewq/math/BVH.h"
#include "litewq/math/Shape.h"
#include "litewq/utils/logging.h"

#include <chrono>
#include <algorithm>

using namespace litewq;
using namespace std::chrono;

static BVHNode *BuildBVH(std::vector<Shape *> shapes,
                         int begin, int end)
{
    CHECK_LT(begin, end);
    BVHNode *Node = new BVHNode;

    Bounds3 NodeBound;
    for (int i = begin; i < end; ++i) {
        NodeBound = Union(NodeBound, shapes[i]->WorldBound());
    }
    if (end - begin == 1) {
        Node->bound = shapes[begin]->WorldBound();
        Node->left = nullptr;
        Node->right = nullptr;
        return Node;
    }
    else if (end - begin == 2) {
        Node->left = BuildBVH(shapes, begin, begin + 1);
        Node->right = BuildBVH(shapes, begin + 1, end);

        Node->bound = Union(Node->left->bound, Node->right->bound);
        return Node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = begin; i < end; ++i) {
            centroidBounds =
                    Union(centroidBounds, shapes[i]->WorldBound().Centroid());
        }
        /* Estimate SAH */
        constexpr int nBuckets = 12;
        struct BucketInfo {
            int count = 0;
            Bounds3 bound;
        };
        BucketInfo buckets[nBuckets];
        int dim = centroidBounds.MaximumExtent();
        float minCost = INFINITY;
        int minCostSplit = 0;
        for (int i = begin; i < end; ++i) {
            Bounds3 shape_bound = shapes[i]->WorldBound();
            int b = nBuckets * centroidBounds.Offset(shape_bound.Centroid())[dim];
            if (b == nBuckets)
                b = nBuckets - 1;
            CHECK_GE(b, 0);
            CHECK_LT(b, nBuckets);
            buckets[b].count++;
            buckets[b].bound = Union(buckets[b].bound, shape_bound);
        }
        /* enumerate split point */
        for (int i = 0; i < nBuckets - 1; ++i) {
            Bounds3 b0, b1;
            int count0 = 0, count1 = 0;
            for (int j = 0; j <= i; ++j) {
                b0 = Union(b0, buckets[j].bound);
                count0 += buckets[j].count;
            }
            for (int j = i+1; j < nBuckets; ++j) {
                b1 = Union(b1, buckets[j].bound);
                count1 += buckets[j].count;
            }
            float cost = 1 +
                      (count0 * b0.SurfaceArea() +
                       count1 * b1.SurfaceArea()) /
                              NodeBound.SurfaceArea();
            if (cost < minCost) {
                minCost = cost;
                minCostSplit = i;
            }
        }


        switch (dim) {
            case 0:
                std::sort(&shapes[begin] , &shapes[end - 1] + 1, [](auto f1, auto f2) {
                    return f1->WorldBound().Centroid().x <
                           f2->WorldBound().Centroid().x;
                });
                break;
            case 1:
                std::sort(&shapes[begin] , &shapes[end - 1] + 1, [](auto f1, auto f2) {
                    return f1->WorldBound().Centroid().y <
                           f2->WorldBound().Centroid().y;
                });
                break;
            case 2:
                std::sort(&shapes[begin] , &shapes[end - 1] + 1, [](auto f1, auto f2) {
                    return f1->WorldBound().Centroid().z <
                           f2->WorldBound().Centroid().z;
                });
                break;
        }

        auto pmid = std::partition(
            &shapes[begin] , &shapes[end - 1] + 1,
            [=](Shape *shape) {
                int b = nBuckets * centroidBounds.Offset(shape->WorldBound().Centroid())[dim];
                if (b == nBuckets)
                    b = nBuckets - 1;
                CHECK_GE(b, 0);
                CHECK_LT(b, nBuckets);
                return b <= minCostSplit;
            }
        );
        int mid = pmid - &shapes[0];

        CHECK_GT(mid, begin);
        CHECK_LT(mid, end);

        Node->left = BuildBVH(shapes, begin, mid);
        Node->right = BuildBVH(shapes, mid, end);

        Node->bound = Union(Node->left->bound, Node->right->bound);
    }

    return Node;
}

BVHUtils::BVHUtils(std::vector<Shape *> &shapes, unsigned int maxShapeInNode) :
    maxShapeInNode(maxShapeInNode), shapes(std::move(shapes))
{
    if (this->shapes.empty())
        return;
    high_resolution_clock::time_point t0,t1;
    double dt0=0.0;

    t0 = high_resolution_clock::now();
    root = BuildBVH(this->shapes, 0, this->shapes.size());
    t1 = high_resolution_clock::now();
    LOG(INFO) << "Building BVH finished: " << duration<double>(t1-t0).count() << " s";
}

bool BVHUtils::intersect(BVHNode *node, const Bounds3 &bbox) const {
    if (!Overlaps(node->bound, bbox)) {
        return false;
    }

    if (node->left == nullptr && node->right == nullptr) {
        return Overlaps(node->bound, bbox);
    }

    bool hit0 = intersect(node->left, bbox);
    bool hit1 = intersect(node->right, bbox);
    return hit0 || hit1;
}