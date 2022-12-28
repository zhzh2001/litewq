#include "litewq/camera/Scene.h"
#include "litewq/mesh/TriMesh.h"

using namespace litewq;

void Scene::render() const {
    for (auto *object : objects) {
        object->render();
    }
}

bool Scene::collision(const litewq::Bounds3 &hitbox) {
    bool hasCollision = false;
    for (auto *object : objects) {
        if (object->bvh)
            hasCollision |= object->bvh->intersect(hitbox);
    }
    return hasCollision;
}