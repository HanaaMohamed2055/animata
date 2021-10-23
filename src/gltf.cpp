#pragma warning(disable : 26451)
#define _CRT_SECURE_NO_WARNINGS

#define CGLTF_IMPLEMENTATION
#include "gltf.h"
#include "Transform.h"
#include "Animation.h"

#include <iostream>
#include <string>

namespace gltf
{
    namespace helper
    {
        math::Transform GetNodeLocalTransform(cgltf_node& node)
        {
            math::Transform result;
            if (node.has_matrix)
            {
                // Decompose matrix
                math::mat4 m(&node.matrix[0]);
                result = math::TransformFromMatrix(m);
            }

            if (node.has_translation)
            {
                result.position = math::vec3(node.translation[0], node.translation[1], node.translation[2]);
            }

            if (node.has_rotation)
            {
                result.rotation = math::Quaternion(node.rotation[0], node.rotation[1], node.rotation[2],
                    node.rotation[3]);
            }

            if (node.has_scale)
            {
                result.scale = math::vec3(node.scale[0], node.scale[1], node.scale[2]);
            }

            return result;
        }

        int GetNodeIndex(cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes)
        {
            if (!target)
            {
                return -1;
            }

            for (unsigned int i = 0; i < numNodes; ++i)
            {
                if (&allNodes[i] == target)
                {
                    return i;
                }
            }

            return -1;
        }

        void GetScalarValues(std::vector<float>& out, const unsigned int compCount, const cgltf_accessor& inAccessor)
        {
            out.resize(compCount + inAccessor.count);

            for (cgltf_size i = 0; i < inAccessor.count; ++i)
            {
                cgltf_accessor_read_float(&inAccessor, i, &out[i * compCount], compCount);
            }
        }

        // Main workhorse here
        template <typename T, int N>
        void TrackFromChannel(animation::Track<T>& result, const cgltf_animation_channel& channel)
        {
            cgltf_animation_sampler& sampler = *channel.sampler;
            result.interpolationKind = animation::INTERPOLATION::CONSTANT;
            if (sampler.interpolation == cgltf_interpolation_type_linear)
            {
                result.interpolationKind = animation::INTERPOLATION::LINEAR;
            }
            else if (sampler.interpolation == cgltf_interpolation_type_cubic_spline)
            {
                result.interpolationKind = animation::INTERPOLATION::CUBIC;
            }

            const bool samplerBicubic = result.interpolationKind == animation::INTERPOLATION::CUBIC ? true : false;

            std::vector<float> times;
            GetScalarValues(times, 1, *sampler.input);

            std::vector<float> val;
            GetScalarValues(val, N, *sampler.output);

            const unsigned int compCount = static_cast<unsigned int>(val.size() / times.size());
            assert(compCount == N);

            const unsigned int numFrames = (unsigned int)sampler.input->count;
            result.frames.resize(numFrames);

            for (unsigned int i = 0; i < numFrames; ++i)
            {
                const unsigned int idx = i * compCount;
                result.frames[i].time = times[i];
                result.frames[i].in = samplerBicubic ? T(val[idx + N]) : 0.0f;
                result.frames[i].value = T(val[idx]);
                result.frames[i].out = samplerBicubic ? T(val[idx + N]) : 0.0f;
            }
        }
    }

    cgltf_data* LoadGLTFFile(const char* filePath)
    {
        cgltf_options options;
        memset(&options, 0, sizeof(cgltf_options));

        cgltf_data* data = NULL;
        cgltf_result result = cgltf_parse_file(&options, filePath, &data);

        if (result != cgltf_result_success)
        {
            std::cout << "Could not load " << filePath << std::endl;
            return nullptr;
        }

        result = cgltf_load_buffers(&options, data, filePath);
        if (result != cgltf_result_success)
        {
            cgltf_free(data);
            std::cout << "Could not load " << filePath << std::endl;
            return nullptr;
        }

        result = cgltf_validate(data);

        if (result != cgltf_result_success)
        {
            cgltf_free(data);
            std::cout << "Invalid file: " << filePath << std::endl;
            return nullptr;
        }

        return data;
    }

    void FreeGLTFFile(cgltf_data*& handle)
    {
        if (!handle)
        {
            std::cout << "WARNING: the handle is already nullptr!" << std::endl;
        }
        else
        {
            cgltf_free(handle);
            handle = nullptr;
        }
    }

    animation::Pose LoadRestPose(cgltf_data* data)
    {
        unsigned int boneCount = (unsigned int)data->nodes_count;
        animation::Pose result;
        result.Resize(boneCount);

        for (unsigned int i = 0; i < boneCount; ++i)
        {
            cgltf_node* node = &(data->nodes[i]);
            math::Transform t = helper::GetNodeLocalTransform(data->nodes[i]);
            result.joints[i] = t;
            int parentIdx = helper::GetNodeIndex(node->parent, data->nodes, boneCount);
            result.parents[i] = parentIdx;
        }
        return result;
    }

    std::vector<std::string> LoadJointNames(cgltf_data* data)
    {
        unsigned int boneCount = (unsigned int)data->nodes_count;
        std::vector<std::string> result(boneCount, "Not set");

        for (unsigned int i = 0; i < boneCount; ++i)
        {
            cgltf_node* node = &(data->nodes[i]);
            if (node->name == nullptr)
            {
                result[i] = "EMPTY NODE";
            }
            else
            {
                result[i] = node->name;
            }
        }
        return result;
    }

    void LoadAnimationClips(std::vector<animation::Clip>& clips, cgltf_data* data)
    {
        unsigned int clipCount = (unsigned int)data->animations_count;
        unsigned int nodeCount = (unsigned int)data->nodes_count;

        clips.resize(clipCount);

        for (unsigned int i = 0; i < clipCount; ++i)
        {
            clips[i].name = data->animations[i].name;
            unsigned int animationChannelCount = (unsigned int)data->animations[i].channels_count;
            for (unsigned int j = 0; j < animationChannelCount; ++j)
            {
                cgltf_animation_channel& channel = data->animations[i].channels[j];
                cgltf_node* target = channel.target_node;
                int nodeIndex = helper::GetNodeIndex(target, data->nodes, nodeCount);

                // TODO : bone id for each transform track
                if (channel.target_path == cgltf_animation_path_type_translation)
                {
                    animation::VectorTrack& track = clips[i][nodeIndex].position;
                    helper::TrackFromChannel<math::vec3, 3>(track, channel);
                }
                else if (channel.target_path == cgltf_animation_path_type_rotation)
                {
                    animation::QuaternionTrack& track = clips[i][nodeIndex].rotation;
                    helper::TrackFromChannel<math::Quaternion, 4>(track, channel);
                }
                else if (channel.target_path == cgltf_animation_path_type_scale)
                {
                    animation::VectorTrack& track = clips[i][nodeIndex].scale;
                    helper::TrackFromChannel<math::vec3, 3>(track, channel);
                }
                // TODO- weights
            }
            clips[i].RecalculateDuration();
            // TOOD - clip looping ?
        }
    }
}