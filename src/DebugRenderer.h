#pragma once
#include "Application.h"
#include "Gfx.h"
#include "Math.h"
#include "Animation.h"
#include "Skinning.h"

struct DebugPose
{
    gfx::VertexBuffer<math::vec3>* mVB;
    std::vector<math::vec3> mPoints;
    animation::Pose mPose;

    DebugPose();
    void Update(const animation::Pose& pose);
    void Draw(gfx::draw::DRAW_MODE drawMode, gfx::Shader* shader, const math::vec3& color, const math::mat4& mvp);
    ~DebugPose();
};

class DebugRenderer : public Application
{
    gfx::Shader* mShader;
    unsigned int mCurrentClip;
    std::vector<animation::Clip> mClips;
    DebugPose* mRestPose;
    DebugPose* mCurrentPose;
    skin::Skeleton mSkeleton;
    float mPlayBackTime;

public:
    void Initialize() override;
    void Update(float inDeltaTime) override;
    void Render(float inAspectRatio) override;
    void Shutdown() override;
};