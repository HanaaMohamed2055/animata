#pragma once

#include "Math.h"

namespace math
{
    inline mat4 frustum(const float l, const float r, const float b, const float t, const float n, const float f)
    {
        if (l == r || t == b || n == f)
        {
            std::cout << "Invalid frustum\n";
            return mat4();
        }

        return mat4(
            (2.0f * n) / (r - l), 0, 0, 0,
            0, (2.0f * n) / (t - b), 0, 0,
            (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
            0, 0, (-2 * f * n) / (f - n), 0
        );
    }

    inline mat4 perspective(const float fov, const float aspectRatio, const float n, const float f)
    {
        // In OpenGL, the fov corresponds to the vertical angle.
        float ymax = n * tanf(fov * PI / 360.0f);
        float xmax = ymax * aspectRatio;
        mat4 m = frustum(-xmax, xmax, -ymax, ymax, n, f);
        transpose(m);
        return m;
    }

    inline mat4 orthographic(const float l, const float r, const float t, const float b, const float n, const float f)
    {
        if (l == r || t == b || n == f)
        {
            return mat4();
        }

        return mat4(2.0f / (r - l), 0, 0, 0,
                    0, 2.0f / (t - b), 0, 0,
                    0, 0, -2.0f / (f - n), 0,
                    -((r + l) / (r - l)), -((t + b) / (t - b)), -((f + n) / (f - n)), 1);
    }

    inline mat4 lookAt(const vec3& position, const vec3& target, const vec3& up)
    {
        vec3 f = normalized(position - target);
        vec3 r = normalized(cross(up, f)); // right hand
        if (r == vec3(0, 0, 0))
        {
            return mat4(); // error, not the best way ever for handling errors
        }

        vec3 u = cross(f, r);

        vec3 t = vec3(
            -dot(r, position),
            -dot(u, position),
            -dot(f, position));

        // Transpose upper 3x3 matrix to get the inverse
        return mat4(
            r.v[0], u.v[0], f.v[0], 0,
            r.v[1], u.v[1], f.v[1], 0,
            r.v[2], u.v[2], f.v[2], 0,
            t.v[0], t.v[1], t.v[2], 1
        );
    }
}

