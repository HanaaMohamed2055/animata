#pragma once
#include "Application.h"
#include "Gfx.h"
#include "Math.h"
#include "Skinning.h"
#include "Animation.h"

struct AnimationInstance
{
    animation::Pose animatedPose;
    std::vector<math::mat4> posePalette;
    unsigned int clip = 0;
    float playback = 0;
    math::Transform model;
};

class SampleRenderer : public Application
{
    gfx::Texture* mDiffuseTexture;
    gfx::Shader* mStaticShader;
    gfx::Shader* mSkinnedShader;
    std::vector<skin::AnimatedMesh> mCPUMeshes;
    std::vector<skin::AnimatedMesh> mGPUMeshes;
    skin::Skeleton mSkeleton;
    std::vector<animation::Clip> mClips;

    AnimationInstance mGPUAnimInfo;
    AnimationInstance mCPUAnimInfo;
public:
    void Initialize() override;
    void Update(float inDeltaTime) override;
    void Render(float inAspectRatio) override;
    void Shutdown() override;
};