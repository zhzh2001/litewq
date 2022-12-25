#include "litewq/math/Shape.h"
#include "litewq/math/BoundingBox.h"
#include "litewq/mesh/TriMesh.h"

using namespace litewq;

Bounds3 Shape::WorldBound() {
    Bounds3 Bound = ObjectBound();
    return Transform(Bound, *ObjectToWorld);
}

Bounds3 Triangle::ObjectBound() {

    return Union(Bounds3(mesh->global_vertices_[v[0]].position_,
                         mesh->global_vertices_[v[1]].position_),
                 mesh->global_vertices_[v[2]].position_);
}