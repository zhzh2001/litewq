#pragma once

#include <cmath>

namespace litewq {


inline int choose_nk(int n, int k) {
    if (k == 0) return 1;
    if (k > n / 2) return choose_nk(n, n - k); 

    long long res = 1; 

    for (int i = 1; i <= k; ++i)
    {
        res *= n - i + 1;
        res /= i;
    }

    return res;
}

inline float bernstein_poly(int n, int i, float u) {
    return choose_nk(n, i) * powf(u, i) * powf(1 - u, n - i); 
}

template <typename T>
inline T Lerp(float t, T v1, T v2) {
    return (1 - t) * v1 + t * v2;
}

} // end namespace litewq
