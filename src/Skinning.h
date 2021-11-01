#pragma once
#include "Animation.h"
#include "Gfx.h"

namespace skin
{
    // Skeleton/Armature/Rig
    struct Skeleton
    {
        animation::Pose restPose;
        animation::Pose bindPose;
        std::vector<math::mat4> inverseBindPose;
        std::vector<std::string> jointNames;

        // Must be called every time bind pose is updated
        void UpdateInverseBindPose();
    };

    struct AnimatedMesh
    {
        std::vector<math::vec3> mPositions;
        gfx::VertexBuffer<math::vec3>* mPositionAttribs;

        std::vector<math::vec3> mNormals;
        gfx::VertexBuffer<math::vec3>* mNormalAttribs;

        std::vector<math::vec2> mTextureCoordinates;
        gfx::VertexBuffer<math::vec2>* mTextureAttribs;

        std::vector<math::vec4> mWeights;
        gfx::VertexBuffer<math::vec4>* mWeightAttribs;

        std::vector<math::ivec4> mInfluences;
        gfx::VertexBuffer<math::ivec4>* mInfluenceAttribs;

        std::vector<unsigned int> mIndices;
        gfx::IndexBuffer* mIndexBuffer;

        // Internal caching for cpu skinning
        // Optional
        std::vector<math::vec3> mSkinnedPositions;
        std::vector<math::vec3> mSkinnedNormals;
        std::vector<math::mat4> mPosePalette;

        AnimatedMesh();
        AnimatedMesh(const AnimatedMesh&);
        ~AnimatedMesh();
        AnimatedMesh& operator=(const AnimatedMesh&);

        void CPUSkin(std::vector<math::mat4>& animatedPose);
        void UpdateGPUBuffers();
        void Bind(int position, int normals, int texcoord, int weight, int influence);
        void UnBind(int position, int normal, int texcoord, int weight, int influence);
        void Draw();
        void DrawInstanced(unsigned int instanceCount);
    };
}