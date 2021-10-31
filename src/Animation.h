#pragma once

#include <vector>
#include "Math.h"
#include "Transform.h"

namespace animation
{
    enum class INTERPOLATION {
        CONSTANT,
        LINEAR,
        CUBIC
    };

    namespace helper
    {
        inline float Interpolate(const float a, const float b, const float t)
        {
            return a + (b - a) * t;
        }

        inline math::vec3 Interpolate(const math::vec3& a, const math::vec3& b, const float t)
        {
            return math::lerp(a, b, t);
        }

        inline math::Quaternion Interpolate(const math::Quaternion& a, const math::Quaternion& b, const float t)
        {
            math::Quaternion result = math::mix(normalized(a), normalized(b), t);
            if (math::dot(a, b) < 0)
            {
                result = math::mix(a, -b, t);
            }

            return math::normalized(result);
        }

        inline void Neighbourhood(const math::Quaternion& a, math::Quaternion& b)
        {
            if (math::dot(a, b))
            {
                b = -b;
            }
        }
    }

    template<typename T>
    inline T Hermite(float t, T& p1, T& s1, T& p2, T& s2)
    {
        const float t2 = t * t;
        const float t3 = t2 * t;
        const float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        const float h2 = -2.0f * t3 + 3.0f * t2;
        const float h3 = t3 - 2.0f * t2 + t;
        const float h4 = t3 - t2;
        return p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
    }

    template<>
    inline math::Quaternion Hermite(float t, math::Quaternion& p1, math::Quaternion& s1, math::Quaternion& p2, math::Quaternion& s2)
    {
        helper::Neighbourhood(p1, p2);
        const float t2 = t * t;
        const float t3 = t2 * t;
        const float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        const float h2 = -2.0f * t3 + 3.0f * t2;
        const float h3 = t3 - 2.0f * t2 + t;
        const float h4 = t3 - t2;
        math::Quaternion result = p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
        return normalized(result);
    }

    template <typename T>
    inline T Bezier(float t, T& p1, T& c1, T& p2, T& c2)
    {
        const float x = (1.0f - t);
        return p1 * (x * x * x) +
            c1 * (3.0f * x * x * t) +
            c2 * (3.0f * x * (t * t)) +
            p2 * (t * t * t);
    }
 
    template <typename T>
    struct Frame
    {
        float time;
        T value;
        T in;  // incoming tanget
        T out; // outgoing tangent
    };

    typedef Frame<float> ScalarFrame;
    typedef Frame<math::vec3> VectorFrame;
    typedef Frame<math::Quaternion> QuaternionFrame;

    template <typename T>
    struct Track
    {
        std::vector<Frame<T>> frames;
        INTERPOLATION interpolationKind;
        
        Track(INTERPOLATION kind = INTERPOLATION::LINEAR)
            :interpolationKind(kind)
        {}

        inline float GetStartTime()
        {
            float t = 0;
            if (!frames.empty())
            {
                t = frames[0].time;
            }
            return t;
        }

        inline float GetEndTime()
        {
            float t = 0;
            if (!frames.empty())
            {
                t = frames[frames.size() - 1].time;
            }
            return t;
        }
    
        inline T Sample(float time, bool looping, INTERPOLATION interpolation = INTERPOLATION::LINEAR)
        {
            T value;
            int currentFrameIndex = GetFrameIndex(time, looping);
            if (currentFrameIndex < 0 || currentFrameIndex >= (int)(frames.size() - 1))
            {
                return value;
            }

            switch (interpolation)
            {
            case INTERPOLATION::CONSTANT:
            {
                value = frames[currentFrameIndex].value;
            }
            break;
            case INTERPOLATION::LINEAR:
            {
                int nextFrameIndex = currentFrameIndex + 1;
                const float trackTime = AdjustTimeToFitTrack(time, looping);
                const float thisTime = frames[currentFrameIndex].time;
                const float frameDelta = frames[nextFrameIndex].time - thisTime;
                if (frameDelta > 0.0f)
                {
                    const float t = (trackTime - thisTime) / frameDelta;
                    value = helper::Interpolate(frames[currentFrameIndex].value, frames[nextFrameIndex].value, t);
                }
            }
            break;
            case INTERPOLATION::CUBIC:
            {
                int nextFrameIndex = currentFrameIndex + 1;
                const float trackTime = AdjustTimeToFitTrack(time, looping);
                const float thisTime = frames[currentFrameIndex].time;
                const float frameDelta = frames[nextFrameIndex].time - thisTime;
                if (frameDelta > 0.0f)
                {
                    const float t = (trackTime - thisTime) / frameDelta;
                    const T& p1 = frames[currentFrameIndex].value;
                    const T& s1 = frames[currentFrameIndex].out;
                    const T& p2 = frames[nextFrameIndex].value;
                    const T& s2 = frames[nextFrameIndex].value;
                    value = Hermite(t, p1, s1, p2, s2);
                }
            }
            break;
            }
            return value;
        }

        inline int GetFrameIndex(float time, bool looping)
        {
            unsigned int size = (unsigned int)frames.size();
            int result = -1;

            if (size < 2)
            {
                return result;
            }

            const float start = frames[0].time;
            if (looping)
            {
                const float end = frames[size - 1].time;
                const float duration = end - start;

                time = fmod(time - start, end - start);

                if (time < 0.0f)
                {
                    time += end - start;
                }
                time += start;
            }
            else
            {
                // Clamp
                if (time <= start)
                {
                    result = 0;
                    return result;
                }

                if (time >= frames[size - 2].time)
                {
                    result = size - 2;
                    return result;
                }
            }

            for (int i = size - 1; i >= 0; --i)
            {
                if (time >= frames[i].time)
                {
                    result = i;
                    break;
                }
            }
            return result;
        }

        // To be called when the playback time of an animation changes
        inline float AdjustTimeToFitTrack(float time, bool looping)
        {
            unsigned int size = (unsigned int)frames.size();
            if (size < 2)
            {
                return 0.0f;
            }

            const float start = frames[0].time;
            const float end = frames[size - 1].time;
            const float duration = end - start;

            if (duration <= 0.0f)
            {
                return 0.0f;
            }

            if (looping)
            {
                time = fmod(time - start, end - start);

                if (time < 0.0f)
                {
                    time += end - start;
                }
                time += start;               
            }
            else
            {
                if (time <= start)
                {
                    time = start;
                }

                if (time >= end)
                {
                    time = end;
                }
            }
            return time;
        }
    };

    typedef Track<float> ScalarTrack;
    typedef Track<math::vec3> VectorTrack;
    typedef Track<math::Quaternion> QuaternionTrack;

    struct TransformTrack
    {
        inline float GetStartTime()
        {
            float result = 0.0f;
            bool resultSet = false;
            if (position.frames.size() > 1)
            {
                result = position.GetStartTime();
                resultSet = true;
            }

            if (rotation.frames.size() > 1)
            {
                float rotationStart = rotation.GetStartTime();
                if (rotationStart < result || resultSet)
                {
                    result = rotationStart;
                    resultSet = true;
                }
            }

            if (scale.frames.size() > 1)
            {
                float scaleStart = scale.GetStartTime();
                if (scaleStart < result || resultSet)
                {
                    result = scaleStart;
                    resultSet = true;
                }
            }

            return result;
        }

        inline float GetEndTime()
        {
            float result = 0.0f;
            bool resultSet = false;
            if (position.frames.size())
            {
                result = position.GetEndTime();
                resultSet = true;
            }

            if (rotation.frames.size())
            {
                float rotationEnd = rotation.GetEndTime();
                if (rotationEnd < result || resultSet)
                {
                    result = rotationEnd;
                    resultSet = true;
                }
            }

            if (scale.frames.size())
            {
                float scaleEnd = scale.GetEndTime();
                if (scaleEnd < result || resultSet)
                {
                    result = scaleEnd;
                    resultSet = true;
                }
            }

            return result;
        }

        inline bool IsValid()
        {
            return ((position.frames.size() > 1) || (rotation.frames.size() > 1) ||
                (scale.frames.size() > 1));
        }

        inline math::Transform Sample(const math::Transform& ref, float time, bool looping)
        {
            math::Transform result = ref;
            if (position.frames.size() > 1)
            {
                result.position = position.Sample(time, looping);
            }

            if (rotation.frames.size() > 1)
            {
                result.rotation = rotation.Sample(time, looping);
            }

            if (scale.frames.size() > 1)
            {
                result.scale = scale.Sample(time, looping);
            }
            return result;
        }

        unsigned int boneID = -1;
        VectorTrack position;
        QuaternionTrack rotation;
        VectorTrack scale;   
    };

    // Represents the state of an animation at a given time.
    struct Pose
    {
        std::vector<math::Transform> joints;
        std::vector<int> parents;

        inline void Resize(unsigned int size)
        {
            joints.resize(size);
            parents.resize(size);
        }

        inline unsigned int Size()
        {
            return (unsigned int)joints.size();
        }

        inline void GetMatrixPalette(std::vector<math::mat4>& out)
        {
            unsigned int size = Size();
            if (size != out.size())
            {
                out.resize(size);
            }

            for (unsigned int i = 0; i < size; ++i)
            {
                math::Transform t = GlobalTransform(i);
                out[i] = MatrixFromTransform(t);
            }
        }

        inline Pose& operator=(const Pose& p)
        {
            if (&p == this)
            {
                return *this;
            }

            if (parents.size() != p.parents.size())
            {
                parents.resize(p.parents.size());
            }

            if (joints.size() != p.joints.size())
            {
                joints.resize(p.joints.size());
            }

            if (parents.size() != 0)
            {
                memcpy(parents.data(), p.parents.data(), sizeof(int) * parents.size());
            }

            if (joints.size() != 0)
            {
                memcpy(joints.data(), p.joints.data(), sizeof(math::Transform) * joints.size());
            }
            return *this;
        }

        inline math::Transform& LocalTransform(unsigned int index)
        {
            return joints[index];
        }

        inline math::Transform GlobalTransform(unsigned int index) const
        {
            math::Transform result = joints[index];                      
            for (int parent = parents[index]; parent >= 0; parent = parents[parent])
            {
                result = combine(joints[parent], result);
            }
            return result;
        }

        inline math::Transform operator[](unsigned int index)
        {
            return GlobalTransform(index);
        }

        inline bool operator==(const Pose& other)
        {
            if (joints.size() != other.joints.size())
            {
                return false;
            }

            if (parents.size() != other.parents.size())
            {
                return false;
            }

            for (unsigned int i = 0; i < joints.size(); ++i)
            {
                if ((joints[i] != other.joints[i]) || (parents[i] != other.parents[i]))
                {
                    return false;
                }
            }

            return true;
        }

        inline bool operator!=(const Pose& other)
        {
            return !(*this == other);
        }
    };

    struct Clip
    {
        std::vector<TransformTrack> tracks;
        std::string name = "No name";
        bool looping = false;
        float startTime = 0.f;
        float endTime = 0.f;

        inline float AdjustTimeToFitRange(float time)
        {
            const float duration = endTime - startTime;

            if (duration <= 0.0f)
            {
                return 0.0f;
            }

            if (looping)
            {
                time = fmod(time - startTime, endTime - startTime);

                if (time < 0.0f)
                {
                    time += endTime - startTime;
                }
                time += startTime;
            }
            else
            {
                if (time <= startTime)
                {
                    time = startTime;
                }

                if (time >= endTime)
                {
                    time = endTime;
                }
            }
            return time;

        }

        inline unsigned int& JointIdAtIndex(unsigned int index)
        {
            return tracks[index].boneID;
        }

        inline TransformTrack& operator[](unsigned int jointIndex)
        {
            for (unsigned int i = 0; i < tracks.size(); ++i)
            {
                if (tracks[i].boneID == jointIndex)
                {
                    return tracks[i];
                }
            }
            TransformTrack track;
            track.boneID = jointIndex;
            tracks.push_back(track);
            return tracks[tracks.size() - 1];
        }

        inline float Sample(Pose& outPose, float time)
        {
            if (GetDuration() == 0.f)
            {
                return 0.f;
            }

            time = AdjustTimeToFitRange(time);
            unsigned int size = static_cast<unsigned int>(tracks.size());
            for (unsigned int i = 0; i < size; ++i)
            {
                unsigned int jointIndex = tracks[i].boneID;
                math::Transform localTransform = outPose.LocalTransform(jointIndex);
                math::Transform animated = tracks[i].Sample(localTransform, time, looping);
                math::Transform& updatedTransform = outPose.LocalTransform(jointIndex);
                updatedTransform = animated;
            }
            return time;
        }

        inline void RecalculateDuration()
        {
            startTime = 0.0f;
            bool startSet = false;
            endTime = 0.0f;
            bool endSet = false;
            for (unsigned int i = 0; i < tracks.size(); ++i)
            {
                if (tracks[i].IsValid())
                {
                    float trackStartTime = tracks[i].GetStartTime();
                    float trackEndTime = tracks[i].GetEndTime();
                    if ((trackStartTime < startTime) || !startSet)
                    {
                        startTime = trackStartTime;
                        startSet = true;
                    }

                    if ((trackEndTime > endTime) || !endSet)
                    {
                        endTime = trackEndTime;
                        endSet = true;
                    }
                }
            }
        }

        inline float GetDuration()
        {
            return endTime - startTime;
        }

        inline unsigned int Size()
        {
            return static_cast<unsigned int>(tracks.size());
        }
    };
}