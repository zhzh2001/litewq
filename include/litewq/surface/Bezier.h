#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace litewq {


class BezierSurface {
public:
    BezierSurface() = delete;
    BezierSurface(unsigned int n_us, unsigned int n_vs, 
                 const std::vector<glm::vec3> &ctrl_pts)
        : n_us(n_us), n_vs(n_vs), ctrl_pts_(ctrl_pts) {}
    [[deprecated("The rendering of Bezier surface largely rely on fixed function pipeline now.")]]
    void render();
    void init();
    glm::vec3 getPoint(float u, float v) const;

    unsigned int n_us, n_vs;
    std::vector<glm::vec3> ctrl_pts_; // v major ctrl 
};

} // end namespace litewq

