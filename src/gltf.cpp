#pragma warning(disable : 26451)
#define _CRT_SECURE_NO_WARNINGS

#define CGLTF_IMPLEMENTATION
#include "gltf.h"
#include "Transform.h"
#include "Animation.h"
#include "Skinning.h"

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
            out.resize(compCount * inAccessor.count);

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
                result.frames[i].value = T(val.data() + idx);
                result.frames[i].out = samplerBicubic ? T(val[idx + N]) : 0.0f;
            }
        }

        void MeshFromAttributes(skin::AnimatedMesh& mesh, cgltf_attribute& attribute, cgltf_skin* skin,
            cgltf_node* nodes, unsigned int nodeCount)
        {
            cgltf_attribute_type attribType = attribute.type;
            cgltf_accessor& accessor = *attribute.data;

            unsigned int componentCount = 0;
            switch (accessor.type)
            {
            case cgltf_type_vec2:
                componentCount = 2;
                break;
            case cgltf_type_vec3:
                componentCount = 3;
                break;
            case cgltf_type_vec4:
                componentCount = 4;
                break;
            default:
                assert(false);
            }

            std::vector<float> values;
            GetScalarValues(values, componentCount, accessor);
            unsigned int accessorCount = static_cast<unsigned int>(accessor.count);
            for (unsigned int i = 0; i < accessorCount; ++i)
            {
                unsigned int index = i * componentCount;
                switch (attribType)
                {
                case cgltf_attribute_type_position:
                    mesh.mPositions.push_back(math::vec3(values[index], values[index + 1], values[index + 2]));
                    break;
                case cgltf_attribute_type_texcoord:
                    mesh.mTextureCoordinates.push_back(math::vec2(values[index], values[index + 1]));
                    break;
                case cgltf_attribute_type_weights:
                    mesh.mWeights.push_back(math::vec4(values[index], values[index + 1], values[index + 2], values[index + 3]));
                    break;
                case cgltf_attribute_type_normal:
                {
                    math::vec3 normal = math::vec3(values[index], values[index + 1], values[index + 2]);
                    if (math::lengthSquared(normal) < 0.0000001f)
                    {
                        normal = math::vec3(0, 1, 0);
                    }
                    mesh.mNormals.push_back(normal);
                }
                    break;
                case cgltf_attribute_type_joints:
                    // Indices are skin relative, relative to the joints array in the gltf file
                    // We need to make them relative to node hierarchy instead.
                    // Add 0.5 to round before casting to integers to avoid errors.
                    math::ivec4 joint((int)(values[index] + 0.5f), (int)(values[index + 1] + 0.5f),
                        (int)(values[index + 2] + 0.5f), (int)(values[index + 3] + 0.5f));
                    for (unsigned int j = 0; j < 4; ++j)
                    {
                        // Make sure that even the invalid nodes have a value of zero
                        joint.v[j] = std::max(GetNodeIndex(skin->joints[joint.v[j]], nodes, nodeCount), 0);
                    }
                    mesh.mInfluences.push_back(joint);
                    break;
                }
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
        return std::move(result);
    }

    // TODO: This needs to be optimized
    animation::Pose LoadBindPose(cgltf_data* data)
    {
        animation::Pose restPose = LoadRestPose(data);
        unsigned int boneCount = restPose.Size();
        std::vector<math::Transform> worldBindPoses(boneCount);
        // Default initialization
        for (unsigned int i = 0; i < boneCount; ++i)
        {
            worldBindPoses[i] = restPose.GlobalTransform(i);
        }

        // Load inverse bind matrices from gltf file and convert them to bind matrices
        // Loop through each skinned mesh in the gltf file
        unsigned int skinCount = (unsigned int)data->skins_count;
        for (unsigned int i = 0; i < skinCount; ++i)
        { 
            cgltf_skin* skin = &(data->skins[i]);
            std::vector<float> invBindMatrices;
            helper::GetScalarValues(invBindMatrices, 16, *skin->inverse_bind_matrices);

            unsigned int jointCount = (unsigned int)skin->joints_count;
            for (unsigned int j = 0; j < jointCount; ++j)
            {
                cgltf_node* jointNode = skin->joints[j];
                unsigned int jointIndex = helper::GetNodeIndex(jointNode, data->nodes, boneCount);
                float* matrix = &(invBindMatrices[j * 16]);
                math::mat4 invBindMatrix = math::mat4(matrix);
                math::mat4 bindMatrix = inverse(invBindMatrix);
                math::Transform bindTransform = TransformFromMatrix(bindMatrix);
                worldBindPoses[jointIndex] = bindTransform;
            }
        }

        // Move each joint to the local space of its parent
        animation::Pose bindPose = restPose;
        for (unsigned int i = 0; i < boneCount; ++i)
        {
            int p = bindPose.parents[i];
            if (p >= 0)
            {
                math::Transform parentTransform = worldBindPoses[p];
                math::Transform currentTransform = worldBindPoses[i];
                currentTransform = math::combine(math::inverse(parentTransform), currentTransform);
                bindPose.joints[i] = currentTransform;
            }
        }
        return bindPose;
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
            clips[i].looping = true;
            // TOOD - clip looping ?
        }
    }

    skin::Skeleton LoadSkeleton(cgltf_data* data)
    {
        skin::Skeleton result;
        result.restPose = LoadRestPose(data);
        result.bindPose = LoadBindPose(data);
        result.jointNames = LoadJointNames(data);
        result.UpdateInverseBindPose();

        return std::move(result);
    }

    // Let's assume for now that the gltf file has only one model/mesh
    void LoadMeshes(std::vector<skin::AnimatedMesh>& meshes, cgltf_data* data)
    {
        cgltf_node* nodes = data->nodes;
        unsigned int nodeCount = static_cast<unsigned int>(data->nodes_count);
        for (unsigned int i = 0; i < nodeCount; ++i)
        {
            cgltf_node* node = &nodes[i];
            if (node->mesh == nullptr || node->skin == nullptr)
            {
                continue;
            }
            unsigned int primitiveCount = static_cast<unsigned int>(node->mesh->primitives_count);
            for (unsigned int j = 0; j < primitiveCount; ++j)
            {
                meshes.push_back(skin::AnimatedMesh());
                skin::AnimatedMesh& mesh = meshes[meshes.size() - 1];
                cgltf_primitive* primitive = &node->mesh->primitives[j];
                unsigned int attributeCount = static_cast<unsigned int>(primitive->attributes_count);
                for (unsigned int k = 0; k < attributeCount; ++k)
                {
                    cgltf_attribute* attribute = &primitive->attributes[k];
                    helper::MeshFromAttributes(mesh, *attribute, node->skin, nodes, nodeCount);
                }
            
                unsigned int indicesCount = static_cast<unsigned int>(primitive->indices->count);
                mesh.mIndices.resize(indicesCount);
                for (unsigned int k = 0; k < indicesCount; ++k)
                {
                    mesh.mIndices[k] = static_cast<unsigned int>(cgltf_accessor_read_index(primitive->indices, k));
                }
                mesh.UpdateGPUBuffers();
            }
        }
    }
}

