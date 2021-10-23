#pragma once

#if _DEBUG
#include <assert.h>
#endif

#include <iostream>
#include <cmath>


namespace math
{
    constexpr static float MY_EPSILON = 0.000001f;
    constexpr static float PI = 3.14159265359f;

    template <typename T>
    struct Tvec2 {
        T v[2];

        inline Tvec2(T x = T(0), T y = T(0))
        {
            v[0] = x;
            v[1] = y;
        }

        inline Tvec2(T* fv[2])
        {
            v[0] = fv[0];
            v[1] = fv[1];
        }
    };

    typedef Tvec2<float> vec2;
    typedef Tvec2<int> ivec2;

    template <typename T>
    struct Tvec4 {
        T v[4];

        inline Tvec4(T x = T(0), T y = T(0), T z = T(0), T w = T(0))
        {
            v[0] = x;
            v[1] = y;
            v[2] = z;
            v[3] = w;
        }

        inline Tvec4(T* fv[4])
        {
            v[0] = fv[0];
            v[1] = fv[1];
            v[2] = fv[2];
            v[3] = fv[3];
        }
    };

    typedef Tvec4<float> vec4;
    typedef Tvec4<int> ivec4;
    typedef Tvec4<unsigned int> uivec4;

    struct vec3 {

        float v[3];

        inline vec3(float x = 0.f, float y = 0.f, float z = 0.f)
        {
            v[0] = x;
            v[1] = y;
            v[2] = z;
        }

        inline vec3(float fv[3])
        {
            v[0] = fv[0];
            v[1] = fv[1];
            v[2] = fv[2];
        }
    };

    inline vec3 operator+(const vec3& l, const vec3& r)
    {
        return vec3(l.v[0] + r.v[0], l.v[1] + r.v[1], l.v[2] + r.v[2]);
    }

    inline vec3 operator-(const vec3& l, const vec3& r)
    {
        return vec3(l.v[0] - r.v[0], l.v[1] - r.v[1], l.v[2] - r.v[2]);
    }

    inline vec3 operator*(const vec3& l, const vec3& r)
    {
        return vec3(l.v[0] * r.v[0], l.v[1] * r.v[1], l.v[2] * r.v[2]);
    }

    inline vec3 operator+(const vec3& v, const float f)
    {
        return vec3(v.v[0] + f, v.v[1] + f, v.v[2] + f);
    }

    inline vec3 operator-(const vec3& v, const float f)
    {
        return vec3(v.v[0] - f, v.v[1] - f, v.v[2] - f);
    }

    inline vec3 operator*(const vec3& v, const float f)
    {
        return vec3(v.v[0] * f, v.v[1] * f, v.v[2] * f);
    }

    inline float dot(const vec3& l, const vec3& r)
    {
        return l.v[0] * r.v[0] + l.v[1] * r.v[1] + l.v[2] * r.v[2];
    }

    inline float lengthSquared(const vec3& v)
    {
        return dot(v, v);
    }

    inline float length(const vec3& v)
    {
        const float lenSq = lengthSquared(v);
        return (lenSq > MY_EPSILON ? sqrtf(lengthSquared(v)) : 0.f);
    }

    inline float distance(const vec3& l, const vec3& r)
    {
        return length(l - r);
    }

    inline void normalize(vec3& v)
    {
        const float len = length(v);
        assert(len > MY_EPSILON);
        if (len <= MY_EPSILON)
            return;

        const float inverseLength = 1 / len;
        v.v[0] *= inverseLength;
        v.v[1] *= inverseLength;
        v.v[2] *= inverseLength;
    }

    inline vec3 normalized(const vec3& v)
    {
        const float len = length(v);
        assert(len > MY_EPSILON);
        if (len <= MY_EPSILON)
            return v;

        const float inverseLength = 1 / len;
        return vec3(v.v[0] * inverseLength, v.v[1] * inverseLength, v.v[2] * inverseLength);
    }

    inline float angle(const vec3& a, const vec3& b)
    {
        const float lenA = length(a);
        const float lenB = length(b);
        if (lenA <= MY_EPSILON || lenB <= MY_EPSILON)
        {
            return 0.0f;
        }
        const float dotAB = dot(a, b);
        return acosf(dotAB / (lenA * lenB));
    }

    inline float radiansToDegrees(const float radians)
    {
        return radians * 180.f / PI;
    }

    inline float degreesToRadians(const float degrees)
    {
        return degrees * PI / 180.f;
    }

    inline vec3 project(const vec3& a, const vec3& b)
    {
        const float lenB = length(b);

        if (lenB <= MY_EPSILON)
        {
            return vec3();
        }

        const float scaleB = dot(a, b) / lenB;
        return b * scaleB;
    }

    inline vec3 reject(const vec3& a, const vec3& b)
    {
        vec3 projection = project(a, b);
        return a - projection;
    }

    inline vec3 reflect(const vec3& a, const vec3& b)
    {
        const float lenB = length(b);

        if (lenB <= MY_EPSILON)
        {
            return vec3();
        }

        const float scaleB = dot(a, b) / lenB;
        vec3 proj2 = b * scaleB * 2;

        return a - proj2;
    }

    inline vec3 cross(const vec3& l, const vec3& r)
    {
        return vec3(l.v[1] * r.v[2] - l.v[2] * r.v[1],
            l.v[2] * r.v[0] - l.v[0] * r.v[2],
            l.v[0] * r.v[1] - l.v[1] * r.v[0]);
    }

    inline vec3 lerp(const vec3& from, const vec3& to, const float t)
    {
        return vec3(
            from.v[0] + (to.v[0] - from.v[0]) * t,
            from.v[1] + (to.v[1] - from.v[1]) * t,
            from.v[2] + (to.v[2] - from.v[2]) * t);
    }

    inline vec3 slerp(const vec3& from, const vec3& to, const float t)
    {
        if (t < 0.01f)
        {
            return lerp(from, to, t);
        }

        vec3 from_n = normalized(from);
        vec3 to_n = normalized(to);
        const float theta = angle(from_n, to_n);
        const float sin_theta = sinf(theta);

        const float a = sinf((1.0f - t) * theta) / sin_theta;
        const float b = sinf(t * theta) / sin_theta;

        return from_n * a + to_n * b;
    }

    inline vec3 nlerp(const vec3& from, const vec3& to, const float t)
    {
        vec3 linear = lerp(from, to, t);
        return normalized(linear);
    }

    inline bool operator==(const vec3& l, const vec3& r)
    {
        vec3 diff(l - r);
        return lengthSquared(diff) <= MY_EPSILON;
    }

    inline bool operator!=(const vec3& l, const vec3& r)
    {
        return !(l == r);
    }

// Matrix
#define M4D(aRow, bCol)                 \
    a.cols[0].v[aRow] * b.cols[bCol].v[0] + \
    a.cols[1].v[aRow] * b.cols[bCol].v[1] + \
    a.cols[2].v[aRow] * b.cols[bCol].v[2] + \
    a.cols[3].v[aRow] * b.cols[bCol].v[3]

#define M4V4D(mRow, v0, v1, v2, v3) \
    m.cols[0].v[mRow] * v0 +        \
    m.cols[1].v[mRow] * v1 +        \
    m.cols[2].v[mRow] * v2 +        \
    m.cols[3].v[mRow] * v3 

#define M4SWAP(x, y)    \
    {float t = x; x = y; y = t;}

#define M4_3x3MINOR(m, c0, c1, c2, r0, r1, r2)                           \
(m.cols[c0].v[r0] * (m.cols[c1].v[r1] * m.cols[c2].v[r2] - m.cols[c1].v[r2] * m.cols[c2].v[r1]) - \
 m.cols[c1].v[r0] * (m.cols[c0].v[r1] * m.cols[c2].v[r2] - m.cols[c0].v[r2] * m.cols[c2].v[r1]) + \
 m.cols[c2].v[r0] * (m.cols[c0].v[r1] * m.cols[c1].v[r2] - m.cols[c0].v[r2] * m.cols[c1].v[r1]))  \

    struct mat4
    {
        vec4 cols[4]; // right(0), up(1), forward(2), position(3)
    
        inline mat4()
        {
            cols[0] = vec4(1, 0, 0, 0);
            cols[1] = vec4(0, 1, 0, 0);
            cols[2] = vec4(0, 0, 1, 0);
            cols[3] = vec4(0, 0, 0, 1);
        }

        inline mat4(float fv[16])
        {
            cols[0] = vec4(fv[0], fv[1], fv[2], fv[3]);
            cols[1] = vec4(fv[4], fv[5], fv[6], fv[7]);
            cols[2] = vec4(fv[8], fv[9], fv[10], fv[11]);
            cols[3] = vec4(fv[12], fv[13], fv[14], fv[15]);
        }

        inline mat4(float _00, float _01, float _02, float _03,
                    float _10, float _11, float _12, float _13,
                    float _20, float _21, float _22, float _23,
                    float _30, float _31, float _32, float _33)
        {
            cols[0] = vec4(_00, _01, _02, _03);
            cols[1] = vec4(_10, _11, _12, _13);
            cols[2] = vec4(_20, _21, _22, _23);
            cols[3] = vec4(_30, _31, _32, _33);
        }
    };

    inline bool operator==(const mat4& a, const mat4& b)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < 4; ++j)
            {
                if (fabsf(a.cols[i].v[j] - b.cols[i].v[j]) > MY_EPSILON)
                {
                    return false;
                }
            }
        }
        return true;
    }

    inline bool operator!=(const mat4& a, const mat4& b)
    {
        return !(a == b);
    }

    inline mat4 operator+(const mat4& a, const mat4& b)
    {
        return mat4(a.cols[0].v[0] + b.cols[0].v[0], a.cols[0].v[1] + b.cols[0].v[1], a.cols[0].v[2] + b.cols[0].v[2], a.cols[0].v[3] + b.cols[0].v[3],
                    a.cols[1].v[0] + b.cols[1].v[0], a.cols[1].v[1] + b.cols[1].v[1], a.cols[1].v[2] + b.cols[1].v[2], a.cols[1].v[3] + b.cols[1].v[3], 
                    a.cols[2].v[0] + b.cols[2].v[0], a.cols[2].v[1] + b.cols[2].v[1], a.cols[2].v[2] + b.cols[2].v[2], a.cols[2].v[3] + b.cols[2].v[3],
                    a.cols[3].v[0] + b.cols[3].v[0], a.cols[3].v[1] + b.cols[3].v[1], a.cols[3].v[2] + b.cols[3].v[2], a.cols[3].v[3] + b.cols[3].v[3]);
    }

    inline mat4 operator*(const mat4& a, const float f)
    {
        return mat4(a.cols[0].v[0] * f, a.cols[0].v[1] * f, a.cols[0].v[2] * f, a.cols[0].v[3] * f,
                    a.cols[1].v[0] * f, a.cols[1].v[1] * f, a.cols[1].v[2] * f, a.cols[1].v[3] * f,
                    a.cols[2].v[0] * f, a.cols[2].v[1] * f, a.cols[2].v[2] * f, a.cols[2].v[3] * f,
                    a.cols[3].v[0] * f, a.cols[3].v[1] * f, a.cols[3].v[2] * f, a.cols[3].v[3] * f);
    }

    inline mat4 operator*(const mat4& a, const mat4& b)
    {
        return mat4(M4D(0, 0), M4D(1, 0), M4D(2, 0), M4D(3, 0), // Col 0
                    M4D(0, 1), M4D(1, 1), M4D(2, 1), M4D(3, 1), // Col 1
                    M4D(0, 2), M4D(1, 2), M4D(2, 2), M4D(3, 2), // Col 2
                    M4D(0, 3), M4D(1, 3), M4D(2, 3), M4D(3, 3)  // Col 3
            );
    }

    inline vec4 operator*(const mat4& m, const vec4& v)
    {
        return vec4(M4V4D(0, v.v[0], v.v[1], v.v[2], v.v[3]),
                    M4V4D(1, v.v[0], v.v[1], v.v[2], v.v[3]),
                    M4V4D(2, v.v[0], v.v[1], v.v[2], v.v[3]),
                    M4V4D(3, v.v[0], v.v[1], v.v[2], v.v[3]));
    }

    inline vec3 transformVector(const mat4& m, const vec3& v)
    {
        return vec3(M4V4D(0, v.v[0], v.v[1], v.v[2], 0.0f),
                    M4V4D(1, v.v[0], v.v[1], v.v[2], 0.0f),
                    M4V4D(2, v.v[0], v.v[1], v.v[2], 0.0f));
    }

    inline vec3 transformPoint(const mat4& m, const vec3& v)
    {
        return vec3(M4V4D(0, v.v[0], v.v[1], v.v[2], 1.0f),
                    M4V4D(1, v.v[0], v.v[1], v.v[2], 1.0f),
                    M4V4D(2, v.v[0], v.v[1], v.v[2], 1.0f));
    }

    inline vec3 transformPoint(const mat4& m, const vec3& v, float& w)
    {
        float w_ = w;
        w = M4V4D(3, v.v[0], v.v[1], v.v[2], w_);

        return vec3(M4V4D(0, v.v[0], v.v[1], v.v[2], w_),
                    M4V4D(1, v.v[0], v.v[1], v.v[2], w_),
                    M4V4D(2, v.v[0], v.v[1], v.v[2], w_));
    }

    inline void transpose(mat4& m)
    {
        M4SWAP(m.cols[0].v[1], m.cols[1].v[0]);
        M4SWAP(m.cols[0].v[2], m.cols[2].v[0]);
        M4SWAP(m.cols[0].v[3], m.cols[3].v[0]);
        M4SWAP(m.cols[1].v[2], m.cols[2].v[1]);
        M4SWAP(m.cols[1].v[3], m.cols[3].v[1]);
        M4SWAP(m.cols[2].v[3], m.cols[3].v[2]);
    }

    inline mat4 transposed(const mat4& m)
    {
        return mat4(
            m.cols[0].v[0], m.cols[1].v[0], m.cols[2].v[0], m.cols[3].v[0],
            m.cols[0].v[1], m.cols[1].v[1], m.cols[2].v[1], m.cols[3].v[1],
            m.cols[0].v[2], m.cols[1].v[2], m.cols[2].v[2], m.cols[3].v[2],
            m.cols[0].v[3], m.cols[1].v[3], m.cols[2].v[3], m.cols[3].v[3]
        );
    }

    inline float determinant(const mat4& m)
    {
        return m.cols[0].v[0] * M4_3x3MINOR(m, 1, 2, 3, 1, 2, 3) -
               m.cols[1].v[0] * M4_3x3MINOR(m, 0, 2, 3, 1, 2, 3) +
               m.cols[2].v[0] * M4_3x3MINOR(m, 0, 1, 3, 1, 2, 3) -
               m.cols[3].v[0] * M4_3x3MINOR(m, 0, 1, 2, 1, 2, 3);
    }

    inline mat4 adjugate(const mat4& m)
    {
        // + - + -
        // - + - +
        // + - + -
        // - + - +

        // Cofactor M[i, j] =  Minor[i, j] * pow(-1, i + j)
        mat4 cofactor;
        cofactor.cols[0].v[0] = M4_3x3MINOR(m, 1, 2, 3, 1, 2, 3);
        cofactor.cols[0].v[1] = -M4_3x3MINOR(m, 1, 2, 3, 0, 2, 3);
        cofactor.cols[0].v[2] = M4_3x3MINOR(m, 1, 2, 3, 0, 1, 3);
        cofactor.cols[0].v[3] = -M4_3x3MINOR(m, 1, 2, 3, 0, 1, 2);

        cofactor.cols[1].v[0] = -M4_3x3MINOR(m, 0, 2, 3, 1, 2, 3);
        cofactor.cols[1].v[1] = M4_3x3MINOR(m, 0, 2, 3, 0, 2, 3);
        cofactor.cols[1].v[2] = -M4_3x3MINOR(m, 0, 2, 3, 0, 1, 3);
        cofactor.cols[1].v[3] = M4_3x3MINOR(m, 0, 2, 3, 0, 1, 2);
    
        cofactor.cols[2].v[0] = M4_3x3MINOR(m, 0, 1, 3, 1, 2, 3);
        cofactor.cols[2].v[1] = -M4_3x3MINOR(m, 0, 1, 3, 0, 2, 3);
        cofactor.cols[2].v[2] = M4_3x3MINOR(m, 0, 1, 3, 0, 1, 3);
        cofactor.cols[2].v[3] = -M4_3x3MINOR(m, 0, 1, 3, 0, 1, 2);

        cofactor.cols[3].v[0] = M4_3x3MINOR(m, 0, 1, 2, 1, 2, 3);
        cofactor.cols[3].v[1] = M4_3x3MINOR(m, 0, 1, 2, 0, 2, 3);
        cofactor.cols[3].v[2] = M4_3x3MINOR(m, 0, 1, 2, 0, 1, 3);
        cofactor.cols[3].v[3] = M4_3x3MINOR(m, 0, 1, 2, 0, 1, 2);
    
        transpose(cofactor);

        return cofactor;
    }

    inline mat4 inverse(const mat4& m)
    {
        const float det = determinant(m);
    
        if (det == 0.0f)
        {
            std::cout << "Matrix determinant is 0\n";
            return mat4();
        }

        mat4 adj = adjugate(m);

        return adj * (1 / det);
    }

    // Quaternion
    struct Quaternion
    {
        float x, y, z, w;

        inline Quaternion(float x_ = 0, float y_ = 0, float z_ = 0, float w_ = 1)
            :x(x_), y(y_), z(z_), w(w_)
        {}

        inline Quaternion(const vec3& v, const float scalar)
            :x(v.v[0]), y(v.v[1]), z(v.v[2]), w(scalar)
        {}

        inline Quaternion(float data[4])
            :x(data[0]), y(data[1]), z(data[2]), w(data[3])
        {}
    };

    inline Quaternion quaternionFromAngleAxis(const float angle, const vec3& axis)
    {
        const float sinAngle = sinf(angle / 2);
        vec3 norm = normalized(axis) * sinAngle;
        return Quaternion(norm, cosf(angle / 2));
    }

    Quaternion quaternionFromTo(const vec3& from, const vec3& to);

    inline vec3 axisFromQuaternion(const Quaternion& quat)
    {
        return normalized(vec3(quat.x, quat.y, quat.z));
    }

    inline const float angleFromQuaternion(const Quaternion& quat)
    {
        return 2.0f * acosf(quat.w);
    }

    inline Quaternion operator+(const Quaternion& l, const Quaternion& r)
    {
        return Quaternion(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
    }

    inline Quaternion operator-(const Quaternion& l, const Quaternion& r)
    {
        return Quaternion(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
    }

    inline Quaternion operator*(const Quaternion& q, const float s)
    {
        return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
    }

    inline Quaternion operator-(const Quaternion& q)
    {
        return Quaternion(-q.x, -q.y, -q.z, -q.w);
    }

    inline bool operator==(const Quaternion& l, const Quaternion& r)
    {
        return (fabsf(l.x - r.x) <= MY_EPSILON && fabsf(l.y - r.y) <= MY_EPSILON && fabsf(l.z - r.z) < MY_EPSILON && fabsf(l.w - r.w) <= MY_EPSILON);
    }

    inline bool operator!=(const Quaternion& l, const Quaternion& r)
    {
        return !(l == r);
    }

    bool sameOrientation(const Quaternion& l, const Quaternion& r);

    float dot(const Quaternion& l, const Quaternion& r);

    float lengthSquared(const Quaternion& q);

    float length(const Quaternion& q);

    void normalize(Quaternion& q);

    Quaternion normalized(const Quaternion& q);

    Quaternion conjugate(const Quaternion& q);

    Quaternion inverse(const Quaternion& q);

    Quaternion operator*(const Quaternion& l, const Quaternion& r);

    vec3 operator*(const Quaternion& q, const vec3& v);

    Quaternion mix(const Quaternion& from, const Quaternion& to, const float t);

    Quaternion nlerp(const Quaternion& from, const Quaternion& to, const float t);

    Quaternion operator^(const Quaternion& q, const float t);

    Quaternion slerp(const Quaternion& from, const Quaternion& to, const float t);

    Quaternion lookRotation(const vec3& direction, const vec3& up);

    mat4 MatrixFromQuaternion(const Quaternion& q);

    Quaternion QuaternionFromMatrix(const mat4& m);

    Quaternion SampleFunction(const Quaternion& a, const Quaternion& b);
}

