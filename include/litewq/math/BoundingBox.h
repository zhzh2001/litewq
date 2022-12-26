
#ifndef LITEWQ_BOUNDINGBOX_H
#define LITEWQ_BOUNDINGBOX_H

#include "litewq/math/BasicFunction.h"

#include <glm/glm.hpp>
#include <limits>

namespace litewq {

class Bounds3 {
public:
    Bounds3() {
        float minNum = std::numeric_limits<float>::lowest();
        float maxNum = std::numeric_limits<float>::max();
        pMin = glm::vec3(maxNum, maxNum, maxNum);
        pMax = glm::vec3(minNum, minNum, minNum);
    }
    Bounds3(const glm::vec3 &p) : pMin(p), pMax(p) {}
    Bounds3(const glm::vec3 &p1, const glm::vec3 &p2)
        : pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
               std::min(p1.z, p2.z)),
          pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
               std::max(p1.z, p2.z)) {
    }
    const glm::vec3 &operator [](int i) const {
        return i == 0 ? pMin : pMax;
    }
    glm::vec3 &operator [](int i) {
        return i == 0 ? pMin : pMax;
    }

    bool operator ==(const Bounds3 &b) const {
        return b.pMin == pMin && b.pMax == pMax;
    }
    bool operator !=(const Bounds3 &b) const {
        return b.pMin != pMin || b.pMax != pMax;
    }
    glm::vec3 Corner(int corner) const {
        return glm::vec3((*this)[(corner & 1)].x,
                         (*this)[(corner & 2) ? 1 : 0].y,
                         (*this)[(corner & 4) ? 1 : 0].z);
    }

    glm::vec3 Centroid() const {
        return pMin * 0.5f + pMax * 0.5f;
    }
    glm::vec3 Diagonal() const { return pMax - pMin; }
    /* max extent axis dim. */
    int maxExtent() const {
        glm::vec3 d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    float SurfaceArea() const {
        glm::vec3 d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }
    float Volume() const {
        glm::vec3 d = Diagonal();
        return d.x * d.y * d.z;
    }
    int MaximumExtent() const {
        glm::vec3 d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }
    glm::vec3 Lerp(const glm::vec3 &t) const {
        return glm::vec3(::litewq::Lerp(t.x, pMin.x, pMax.x),
                         ::litewq::Lerp(t.y, pMin.y, pMax.y),
                         ::litewq::Lerp(t.z, pMin.z, pMax.z));
    }
    glm::vec3 Offset(const glm::vec3 &p) const {
        glm::vec3 o = p - pMin;
        if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
        return o;
    }
//    void BoundingSphere(glm::vec3 *center, float *radius) const {
//        *center = (pMin + pMax) / 2;
//        *radius = Inside(*center, *this) ? Distance(*center, pMax) : 0;
//    }
//
//    bool IntersectP(const Ray &ray, Float *hitt0 = nullptr, Float *hitt1 = nullptr) const;
//    inline bool IntersectP(const Ray &ray, const Vector3f &invDir,
//                           const int dirIsNeg[3]) const;
    glm::vec3 pMin, pMax;
};

inline Bounds3
Union(const Bounds3 &b, const glm::vec3 &p) {
    return Bounds3(glm::vec3(std::min(b.pMin.x, p.x),
                             std::min(b.pMin.y, p.y),
                             std::min(b.pMin.z, p.z)),
                   glm::vec3(std::max(b.pMax.x, p.x),
                             std::max(b.pMax.y, p.y),
                             std::max(b.pMax.z, p.z)));
}

inline Bounds3
Union(const Bounds3 &b1, const Bounds3 &b2) {
    return Bounds3(glm::vec3(std::min(b1.pMin.x, b2.pMin.x),
                             std::min(b1.pMin.y, b2.pMin.y),
                             std::min(b1.pMin.z, b2.pMin.z)),
                   glm::vec3(std::max(b1.pMax.x, b2.pMax.x),
                             std::max(b1.pMax.y, b2.pMax.y),
                             std::max(b1.pMax.z, b2.pMax.z)));
}

inline Bounds3
Intersect(const Bounds3 &b1, const Bounds3 &b2) {
    return Bounds3(glm::vec3(std::max(b1.pMin.x, b2.pMin.x),
                             std::max(b1.pMin.y, b2.pMin.y),
                             std::max(b1.pMin.z, b2.pMin.z)),
                   glm::vec3(std::min(b1.pMax.x, b2.pMax.x),
                             std::min(b1.pMax.y, b2.pMax.y),
                             std::min(b1.pMax.z, b2.pMax.z)));
}

inline
bool Overlaps(const Bounds3 &b1, const Bounds3 &b2) {
    bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
    bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
    bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
    return (x && y && z);
}

inline
bool Inside(const glm::vec3 &p, const Bounds3 &b) {
    return (p.x >= b.pMin.x && p.x <= b.pMax.x &&
            p.y >= b.pMin.y && p.y <= b.pMax.y &&
            p.z >= b.pMin.z && p.z <= b.pMax.z);
}

inline Bounds3
Expand(const Bounds3 &b, float delta) {
    return Bounds3(b.pMin - glm::vec3 (delta, delta, delta),
                   b.pMax + glm::vec3 (delta, delta, delta));
}

inline Bounds3
Transform(const Bounds3 &b, const glm::mat4 &M)  {
    Bounds3 ret(glm::vec3(M * glm::vec4(b.pMin.x, b.pMin.y, b.pMin.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMax.x, b.pMin.y, b.pMin.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMin.x, b.pMax.y, b.pMin.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMin.x, b.pMin.y, b.pMax.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMin.x, b.pMax.y, b.pMax.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMax.x, b.pMax.y, b.pMin.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMax.x, b.pMin.y, b.pMax.z, 1.0f)));
    ret = Union(ret, glm::vec3(M * glm::vec4(b.pMax.x, b.pMax.y, b.pMax.z, 1.0f)));
    return ret;
}

} // end namespace litewq


#endif // LITEWQ_BOUNDINGBOX_H
