
#ifndef LITEWQ_SCENE_H
#define LITEWQ_SCENE_H

#include "litewq/math/BoundingBox.h"
#include <vector>

namespace litewq {

class TriMesh;

class Scene {
public:
    /* collision detection */
    bool collision(const Bounds3 &hitbox);
    void render() const;
    void addObject(TriMesh *mesh) {
        objects.push_back(mesh);
    }
    std::vector<TriMesh *> objects;
};

} // end namespace litewq

#endif//LITEWQ_SCENE_H
