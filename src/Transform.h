#pragma once

#include "Math.h"

namespace math
{
    struct Transform
    {
        vec3 position;
        Quaternion rotation;
        vec3 scale;

        Transform(const vec3& p = vec3(0, 0, 0), const Quaternion& r = Quaternion(), const vec3& s = vec3(1, 1, 1))
            :position(p), rotation(r), scale(s)
        {}
    };

    inline Transform combine(const Transform& l, const Transform& r)
    {
        // right-to-left order for consistency
        Transform result;
        result.scale = r.scale * l.scale;
        result.rotation = r.rotation * l.rotation;
        result.position = l.rotation * (l.scale * r.position);
        result.position = l.position + result.position;

        return result;
    }

    inline Transform inverse(const Transform& t)
    {
        Transform result;
        result.rotation = inverse(t.rotation);
        result.scale = vec3(
            (fabs(t.scale.v[0]) < MY_EPSILON) ? 0.0f : 1 / t.scale.v[0],
            (fabs(t.scale.v[1]) < MY_EPSILON) ? 0.0f : 1 / t.scale.v[1],
            (fabs(t.scale.v[2]) < MY_EPSILON) ? 0.0f : 1 / t.scale.v[2]);
        result.position = result.rotation * (result.scale  * (result.position * -1.0f));
        
        return result;
    }

    // Linear blend between transforms 
    inline Transform mix(const Transform& s, const Transform& e, const float t)
    {
        Transform result;
        // Neighbourhooding problem
        Quaternion eRot = e.rotation;
        if (dot(s.rotation, eRot) < 0.0f)
        {
            eRot = -eRot;
        }

        return Transform(lerp(s.position, e.position, t),
                        nlerp(s.rotation, e.rotation, t),
                        lerp(s.scale, e.scale, t));
    }

    inline mat4 MatrixFromTransform(const Transform& t)
    {
        vec3 x = t.rotation * vec3(1, 0, 0);
        vec3 y = t.rotation * vec3(0, 1, 0);
        vec3 z = t.rotation * vec3(0, 0, 1);
        x =  x * t.scale;
        x =  y * t.scale;
        x =  z * t.scale;
        
        vec3 p = t.position;
        return mat4(x.v[0], x.v[1], x.v[2], 0,
                    y.v[0], y.v[1], y.v[2], 0,
                    z.v[0], z.v[1], z.v[2], 0,
                    p.v[0], p.v[1], p.v[2], 1);
    }

    inline Transform TransformFromMatrix(const math::mat4& m)
    {
        Transform result;
        result.rotation = QuaternionFromMatrix(m);
        result.position = vec3(m.cols[3].v[0], m.cols[3].v[1], m.cols[3].v[2]);
        mat4 rotScaleMatrix = mat4(m.cols[0].v[0], m.cols[0].v[1], m.cols[0].v[2], 0,
                                               m.cols[1].v[0], m.cols[1].v[1], m.cols[1].v[2], 0,
                                               m.cols[2].v[0], m.cols[2].v[1], m.cols[2].v[2], 0,
                                               0, 0, 0, 1);
        mat4 inverseRotMatrix = MatrixFromQuaternion(inverse(result.rotation));
        mat4 scaleSkewMat = rotScaleMatrix * inverseRotMatrix;
        result.scale = vec3(scaleSkewMat.cols[0].v[0], scaleSkewMat.cols[1].v[1], scaleSkewMat.cols[2].v[2]);
        return result;
    }

    inline vec3 TransformPoint(const Transform& t, const vec3& p)
    {
        vec3 result;
        result = t.rotation * (p * t.scale);
        result = result + t.position;
        return result;
    }

    inline vec3 TransformVector(const Transform& t, const vec3& p)
    {
        vec3 result;
        result = t.rotation * (p * t.scale);
        return result;
    }

    inline bool operator==(const Transform& t1, const Transform& t2)
    {
        return (t1.position == t2.position && t1.rotation == t2.rotation &&
            t1.scale == t2.scale);
    }

    inline bool operator!=(const Transform& t1, const Transform& t2)
    {
        return !operator==(t1, t2);
    }
}

