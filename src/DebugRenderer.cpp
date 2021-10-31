#include "DebugRenderer.h"

#include "gltf.h"
#include "Camera.h"

using namespace math;

DebugPose::DebugPose()
{    
    mVB = new gfx::VertexBuffer<math::vec3>(mPoints);
}

void DebugPose::Update(const animation::Pose& pose)
{
    mPose = pose;
    mPoints.clear();
    unsigned int jointCount = (unsigned int)pose.joints.size();
    for (unsigned int i = 0; i < jointCount; ++i)
    {
        if (pose.parents[i] > 0)
        {
            mPoints.push_back(pose.GlobalTransform(i).position);
            mPoints.push_back(pose.GlobalTransform(pose.parents[i]).position);
        }
    }
    if (!mPoints.empty())
    {
        mVB->Upload(mPoints);
    }
}

void DebugPose::Draw(gfx::draw::DRAW_MODE drawMode, gfx::Shader* shader, const math::vec3& color, const math::mat4& mvp)
{
    shader->Bind();
    gfx::uniform::Update(shader->GetUniform("mvp"), mvp);
    gfx::uniform::Update(shader->GetUniform("color"), color);
    mVB->Bind(shader->GetAttribute("position"));

    gfx::draw::Draw(static_cast<unsigned int>(mPoints.size()), drawMode);

    mVB->UnBind(shader->GetAttribute("position"));
    shader->unBind();
}

DebugPose::~DebugPose()
{
    delete mVB;
}

void DebugRenderer::Initialize()
{
    // Shaders
    std::string vs = R"(
        #version 460 core
        uniform mat4 mvp;
        in vec3 position;
        void main()
        {
            gl_Position = mvp * vec4(position, 1.0);
        }
    )";

    std::string ps = R"(
        #version 460 core
        uniform vec3 color;
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(color, 1.);
        }
    )";

    mShader = new gfx::Shader(vs, ps);
    
    cgltf_data* data = gltf::LoadGLTFFile("D:/projects/animation_system/assets/Woman.gltf");
    gltf::LoadAnimationClips(mClips, data);

    mRestPose = new DebugPose();
    mCurrentPose = new DebugPose();

    animation::Pose restPose = gltf::LoadRestPose(data);
    mRestPose->Update(restPose);
    mCurrentPose->Update(restPose);

    mCurrentClip = 0;
    for (unsigned int i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i].name == "Walking")
        {
            mCurrentClip = i;
            break;
        }
    }
    gltf::FreeGLTFFile(data);
}

void DebugRenderer::Update(float inDeltaTime)
{    
    mPlayBackTime = mClips[mCurrentClip].Sample(mCurrentPose->mPose, mPlayBackTime + inDeltaTime);
    mCurrentPose->Update(mCurrentPose->mPose);
}

void DebugRenderer::Render(float inAspectRatio)
{
    mat4 projection = transposed(perspective(60.0f, inAspectRatio, 0.01f, 1000.0f));
    mat4 view = lookAt(vec3(0, 4, 7), vec3(0, 4, 0), vec3(0, 1, 0));
    mat4 mvp = projection * view;

    mRestPose->Draw(gfx::draw::DRAW_MODE::LINES, mShader, vec3(1, 0, 0), mvp);
    mCurrentPose->Draw(gfx::draw::DRAW_MODE::LINES, mShader, vec3(0, 0, 1), mvp);
}

void DebugRenderer::Shutdown()
{
    delete mShader;
    delete mRestPose;
    delete mCurrentPose;
    mClips.clear();
}