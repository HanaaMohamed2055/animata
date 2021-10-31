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
        mWeightAttribs = other.mWeightAttribs;
        mInfluenceAttribs = other.mInfluenceAttribs;
        mIndices = other.mIndices;
        UpdateGPUBuffers();
        return *this;
    }

    void AnimatedMesh::CPUSkin(Skeleton& skeleton, animation::Pose& outPose)
    {
        unsigned int vertexCount = static_cast<unsigned int>(mPositions.size());
        if (vertexCount == 0)
        {
            return;
        }

        mSkinnedPositions.resize(vertexCount, vec3());
        mSkinnedNormals.resize(vertexCount, vec3());
        animation::Pose& bindPose = skeleton.bindPose;

        for (unsigned int i = 0; i < vertexCount; ++i)
        {
            vec4& weights = mWeights[i];
            ivec4& joint = mInfluences[i];

            math::Transform skin[4];
            for (unsigned int j = 0; j < 4; ++j)
            {
                // Combine the inverse bind transform of the joint with the animated pose.
                skin[j] = math::combine(outPose.GlobalTransform(joint.v[j]), inverse(bindPose.GlobalTransform(joint.v[j])));
                math::vec3 p = math::TransformPoint(skin[j], mPositions[i]);
                math::vec3 n = math::TransformVector(skin[j], mNormals[i]);
                // blend between 4 influencing joints.
                mSkinnedPositions[i] = mSkinnedPositions[i] + p * weights.v[j];
                mSkinnedNormals[i] = mSkinnedNormals[i] + n * weights.v[j];
            }
        }
        mPositionAttribs->Upload(mSkinnedPositions);
        mNormalAttribs->Upload(mSkinnedNormals);
    }

    void AnimatedMesh::UpdateGPUBuffers()
    {
        if (!mPositions.empty())
        {
            if (!mPositionAttribs)
            {
                mPositionAttribs = new VertexBuffer<vec3>({});
            }
            mPositionAttribs->Upload(mPositions);
        }

        if (!mNormals.empty())
        {
            if (!mNormalAttribs)
            {
                mNormalAttribs = new VertexBuffer<vec3>({});
            }
            mNormalAttribs->Upload(mNormals);
        }

        if (!mTextureCoordinates.empty())
        {
            if (!mTextureAttribs)
            {
                mTextureAttribs = new VertexBuffer<vec2>({});
            }
            mTextureAttribs->Upload(mTextureCoordinates);
        }

        if (!mWeights.empty())
        {
            if (!mWeightAttribs)
            {
                mWeightAttribs = new VertexBuffer<vec4>({});
            }
            mWeightAttribs->Upload(mWeights);
        }

        if (!mInfluences.empty())
        {
            if (!mInfluenceAttribs)
            {
                mInfluenceAttribs = new VertexBuffer<ivec4>({});
            }
            mInfluenceAttribs->Upload(mInfluences);
        }

        if (!mIndices.empty())
        {
            if (!mIndexBuffer)
            {
                mIndexBuffer = new IndexBuffer({});
            }
            mIndexBuffer->Update(mIndices);
        }
    }

    void AnimatedMesh::Bind(unsigned int position, unsigned int normal, unsigned int texcoord,
        unsigned int weight, unsigned int influence)
    {
        mPositionAttribs->Bind(position);
        mNormalAttribs->Bind(normal);
        mTextureAttribs->Bind(texcoord);
        mWeightAttribs->Bind(weight);
        mInfluenceAttribs->Bind(influence);
    }

    void AnimatedMesh::UnBind(unsigned int position, unsigned int normal, unsigned int texcoord,
        unsigned int weight, unsigned int influence)
    {
        mPositionAttribs->UnBind(position);
        mNormalAttribs->Bind(normal);
        mTextureAttribs->Bind(texcoord);
        mWeightAttribs->Bind(weight);
        mInfluenceAttribs->Bind(influence);
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