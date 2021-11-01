#pragma once

#include "Math.h"

namespace math
{
    // Quaternion
    Quaternion quaternionFromTo(const vec3& from, const vec3& to)
    {
        vec3 f = normalized(from);
        vec3 t = normalized(to);

        if (f == t)
        {
            return Quaternion();
        }

        // There is no unique half vector in this case
        if (f == t * -1.f)
        {
            // Take the most orthogonal basis vector
            vec3 ortho(1, 0, 0);

            if (fabsf(f.v[1]) < fabsf(f.v[0]))
            {
                ortho = vec3(0, 1, 0);
            }

            if (fabsf(f.v[2]) < fabsf(f.v[1]) && fabsf(f.v[2]) < fabsf(f.v[0]))
            {
                ortho = vec3(0, 0, 1);
            }

            vec3 axis = normalized(cross(f, ortho));

            return Quaternion(axis, 0.f);
        }

        // Get the halfway vector
        vec3 half = normalized(f + t);
        vec3 axis = cross(f, half);
        return Quaternion(axis, dot(f, half));
    }

    bool sameOrientation(const Quaternion& l, const Quaternion& r)
    {
        return ((fabsf(l.x - r.x) <= MY_EPSILON && fabsf(l.y - r.y) <= MY_EPSILON && fabsf(l.z - r.z) < MY_EPSILON && fabsf(l.w - r.w) <= MY_EPSILON) ||
            (fabsf(l.x + r.x) <= MY_EPSILON && fabsf(l.y + r.y) <= MY_EPSILON && fabsf(l.z + r.z) < MY_EPSILON && fabsf(l.w + r.w) <= MY_EPSILON));
    }

    float dot(const Quaternion& l, const Quaternion& r)
    {
        return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
    }

    float lengthSquared(const Quaternion& q)
    {
        return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    }

    float length(const Quaternion& q)
    {
        float lenSq = lengthSquared(q);

        if (lenSq <= MY_EPSILON)
        {
            return 0.0f;
        }
        return sqrtf(lenSq);
    }

    void normalize(Quaternion& q)
    {
        const float len = length(q);
        if (len <= MY_EPSILON)
        {
            return;
        }
        float i_len = 1.0f / len;

        q.x *= i_len;
        q.y *= i_len;
        q.z *= i_len;
        q.w *= i_len;
    }

    Quaternion normalized(const Quaternion& q)
    {
        const float len = length(q);
        if (len <= MY_EPSILON)
        {
            return Quaternion();
        }

        return Quaternion(q.x / len, q.y / len, q.z / len, q.w / len);
    }

    Quaternion conjugate(const Quaternion& q)
    {
        return Quaternion(-q.x, -q.y, -q.z, q.w);
    }

    Quaternion inverse(const Quaternion& q)
    {
        const float lenSq = lengthSquared(q);
        if (lenSq <= MY_EPSILON)
        {
            return Quaternion();
        }

        const float recip = 1 / lenSq;
        return Quaternion(-q.x * recip, -q.y * recip, -q.z * recip, q.w * recip);
    }

    Quaternion operator*(const Quaternion& l, const Quaternion& r)
    {
        //Quaternion result;
        //vec3 lv(l.x, l.y, l.z);
        //vec3 rv(r.x, r.y, r.z);
        //const float fres = r.w * l.w - dot(rv, lv);

        //vec3 vres = (lv * r.w + rv * l.w) + cross(rv, lv);

        //return Quaternion(vres, fres);

        // This implementation is more performant
        return Quaternion(r.x * l.w + r.y * l.z - r.z * l.y + r.w * l.x,
                        -r.x * l.z + r.y * l.w + r.z * l.x + r.w * l.y,
                        r.x * l.y - r.y * l.x + r.z * l.w + r.w * l.z,
                        -r.x * l.x - r.y * l.y - r.z * l.z + r.w * l.w);
    }

    vec3 operator*(const Quaternion& q, const vec3& v)
    {
        // TODO-Hanaa: we need to have a look at Rodriguez formula
        vec3 qv(q.x, q.y, q.z);

        vec3 v1 = qv * 2.0f * dot(qv, v);
        vec3 v2 = v * (q.w * q.w - dot(qv, qv));
        vec3 v3 = cross(qv, v) * 2.0f * q.w;

        return v1 + v2 + v3;
    }

    Quaternion mix(const Quaternion& from, const Quaternion& to, const float t)
    {
        return from * (1.0f - t) + to * t;
    }

    Quaternion nlerp(const Quaternion& from, const Quaternion& to, const float t)
    {
        return normalized(from + (to - from) * t);
    }

    Quaternion operator^(const Quaternion& q, const float t)
    {
        // TODO-hanaa: do we really need 2?
        const float angle = 2.0f * acosf(q.w);
        vec3 axis(q.x, q.y, q.z);
        normalize(axis);
        
        const float halfCos = cosf(t * angle * 0.5f);
        const float halfSin = sinf(t * angle * 0.5f);

        axis = axis * halfSin;
        return Quaternion(axis, halfCos);
    }

    Quaternion slerp(const Quaternion& from, const Quaternion& to, const float t)
    {
        const float d = fabs(dot(from, to));
        if (d > 1.0f - MY_EPSILON)
        {
            return nlerp(from, to, t);
        }

        Quaternion delta = inverse(from) * to;
        return normalized((delta ^ t) * from);
    }

    Quaternion lookRotation(const vec3& direction, const vec3& up)
    {
        // look at and look rotation are the inverse of each other
        vec3 f = normalized(direction); // object forward
        vec3 u = normalized(up); // desired up

        vec3 r = cross(u, f); // object right
        u = cross(f, r); // object up

        Quaternion WorldToObject = quaternionFromTo(vec3(0, 0, 1), f);
        vec3 objectUp = WorldToObject * vec3(0, 1, 0);

        Quaternion u2u = quaternionFromTo(objectUp, u);

        Quaternion result = WorldToObject * u2u;

        normalize(result);

        return result;
    }

    mat4 MatrixFromQuaternion(const Quaternion& q)
    {
        const vec3 r = q * vec3(1, 0, 0);
        const vec3 u = q * vec3(0, 1, 0);
        const vec3 f = q * vec3(0, 0, 1);

        return mat4(r.v[0], r.v[1], r.v[2], 0,
                    u.v[0], u.v[1], u.v[2], 0,
                    f.v[0], f.v[1], f.v[2], 0,
                    0, 0, 0, 1);
    }

    Quaternion QuaternionFromMatrix(const mat4& m)
    {
        vec3 up = vec3(m.cols[1].v[0], m.cols[1].v[1], m.cols[1].v[2]);
        normalize(up);
        vec3 forward = vec3(m.cols[2].v[0], m.cols[2].v[1], m.cols[2].v[2]);
        normalize(forward);

        vec3 right = cross(up, forward);
        up = cross(forward, right);

        return lookRotation(forward, up);
    }

    Quaternion SampleFunction(const Quaternion& a, const Quaternion& b)
    {
        Quaternion b_ = b;
        if (dot(a, b) < 0.0f)
        {
            b_ = -b;
        }

        return slerp(a, b_, 0.5f);
    }
}

