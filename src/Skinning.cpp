#include "Skinning.h"

#include <string>
#include <vector>

using namespace math;
using namespace gfx;
namespace skin
{
    void Skeleton::UpdateInverseBindPose()
    {
        unsigned int jointCount = static_cast<unsigned int>(bindPose.Size());
        inverseBindPose.resize(jointCount);
        for (unsigned int i = 0; i < jointCount; ++i)
        {
            Transform world = bindPose.GlobalTransform(i);
            inverseBindPose[i] = inverse(MatrixFromTransform(world));
        }
    }

    // Animated mesh
    AnimatedMesh::AnimatedMesh()
    {
        mPositionAttribs = new VertexBuffer<vec3>({});
        mNormalAttribs = new VertexBuffer<vec3>({});
        mTextureAttribs = new VertexBuffer<vec2>({});
        mWeightAttribs = new VertexBuffer<vec4>({});
        mInfluenceAttribs = new VertexBuffer<ivec4>({});
        mIndexBuffer = new IndexBuffer({});
    }

    AnimatedMesh::AnimatedMesh(const AnimatedMesh& other)
    {
        mPositionAttribs = new VertexBuffer<vec3>({});
        mNormalAttribs = new VertexBuffer<vec3>({});
        mTextureAttribs = new VertexBuffer<vec2>({});
        mWeightAttribs = new VertexBuffer<vec4>({});
        mInfluenceAttribs = new VertexBuffer<ivec4>({});
        mIndexBuffer = new IndexBuffer({});
        *this = other;
    }

    AnimatedMesh::~AnimatedMesh()
    {
        delete mPositionAttribs;
        delete mNormalAttribs;
        delete mTextureAttribs;
        delete mWeightAttribs;
        delete mInfluenceAttribs;
        delete mIndexBuffer;
    }

    AnimatedMesh& AnimatedMesh::operator=(const AnimatedMesh& other)
    {
        if (this == &other)
        {
            return *this;
        }

        mPositions = other.mPositions;
        mNormals = other.mNormals;
        mTextureCoordinates = other.mTextureCoordinates;
        mWeights = other.mWeights;
        mInfluences = other.mInfluences;
        mIndices = other.mIndices;
        UpdateGPUBuffers();
        return *this;
    }

    void AnimatedMesh::CPUSkin(std::vector<math::mat4>& animatedPose)
    {
        unsigned int vertexCount = static_cast<unsigned int>(mPositions.size());
        if (vertexCount == 0)
        {
            return;
        }

        mSkinnedPositions.resize(vertexCount, vec3());
        mSkinnedNormals.resize(vertexCount, vec3());

        math::mat4 finalSkinMatrix;
        for (unsigned int i = 0; i < vertexCount; ++i)
        {
            vec4& w = mWeights[i];
            ivec4& j = mInfluences[i];

            math::mat4 skin[4];
            // Combine the inverse bind transform of the joint with the animated pose.
            skin[0] = animatedPose[j.v[0]] * w.v[0];
            skin[1] = animatedPose[j.v[1]] * w.v[1];
            skin[2] = animatedPose[j.v[2]] * w.v[2];
            skin[3] = animatedPose[j.v[3]] * w.v[3];

            // Blend between 4 influencing joints.
            finalSkinMatrix = skin[0] + skin[1] + skin[2] + skin[3];
            mSkinnedPositions[i] = TransformPoint(finalSkinMatrix, mPositions[i]);
            mSkinnedNormals[i] = TransformVector(finalSkinMatrix, mNormals[i]);
        }
        mPositionAttribs->Upload(mSkinnedPositions);
        mNormalAttribs->Upload(mSkinnedNormals);
    }

    void AnimatedMesh::UpdateGPUBuffers()
    {
        if (!mPositions.empty())
        {
            mPositionAttribs->Upload(mPositions);
        }

        if (!mNormals.empty())
        {
            mNormalAttribs->Upload(mNormals);
        }

        if (!mTextureCoordinates.empty())
        {
            mTextureAttribs->Upload(mTextureCoordinates);
        }

        if (!mWeights.empty())
        {
            mWeightAttribs->Upload(mWeights);
        }

        if (!mInfluences.empty())
        {
            mInfluenceAttribs->Upload(mInfluences);
        }

        if (!mIndices.empty())
        {
            mIndexBuffer->Update(mIndices);
        }
    }

    void AnimatedMesh::Bind(int position, int normal, int texcoord, int weight, int influence)
    {
        if (position >= 0)
        {
            mPositionAttribs->Bind(position);
        }
        if (normal >= 0)
        {
            mNormalAttribs->Bind(normal);
        }
        if (texcoord >= 0)
        {
            mTextureAttribs->Bind(texcoord);
        }
        if (weight >= 0)
        {
            mWeightAttribs->Bind(weight);
        }
        if (influence >= 0)
        {
            mInfluenceAttribs->Bind(influence);
        }
    }

    void AnimatedMesh::UnBind(int position, int normal, int texcoord, int weight, int influence)
    {
        if (position >= 0)
        {
            mPositionAttribs->UnBind(position);
        }
        if (normal >= 0)
        {
            mNormalAttribs->Bind(normal);
        }
        if (texcoord >= 0)
        {
            mTextureAttribs->Bind(texcoord);
        }
        if (weight >= 0)
        {
            mWeightAttribs->Bind(weight);
        }
        if (influence >= 0)
        {
            mInfluenceAttribs->Bind(influence);
        }
    }

    void AnimatedMesh::Draw()
    {
        if (!mIndices.empty())
        {
            draw::Draw(*mIndexBuffer, draw::DRAW_MODE::TRIANGLES);
        }
        else
        {
            draw::Draw(static_cast<unsigned int>(mPositions.size()), draw::DRAW_MODE::TRIANGLES);
        }
    }

    void AnimatedMesh::DrawInstanced(unsigned int instanceCount)
    {
        if (!mIndices.empty())
        {
            draw::DrawInstanced(*mIndexBuffer, draw::DRAW_MODE::TRIANGLES, instanceCount);
        }
        else
        {
            draw::DrawInstanced(static_cast<unsigned int>(mPositions.size()), draw::DRAW_MODE::TRIANGLES, instanceCount);
        }
    }
}