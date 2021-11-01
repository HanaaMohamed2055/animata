#include "SampleRenderer.h"
#include "Camera.h"
#include "gltf.h"

using namespace gfx;
using namespace math;

#define DEG2RAD 0.0174533f

void SampleRenderer::Initialize()
{
    cgltf_data* gltf = gltf::LoadGLTFFile("D:/projects/animation_system/assets/Woman.gltf");
    gltf::LoadMeshes(mCPUMeshes, gltf);
    mSkeleton = gltf::LoadSkeleton(gltf);
    gltf::LoadAnimationClips(mClips, gltf);
    gltf::FreeGLTFFile(gltf);

    mGPUMeshes = mCPUMeshes;
    for (auto& mesh : mGPUMeshes)
    {
        mesh.UpdateGPUBuffers();
    }

    mStaticShader = new Shader("D:/projects/animation_system/src/Shaders/static.vert", "D:/projects/animation_system/src/Shaders/lit.frag");
    mSkinnedShader = new Shader("D:/projects/animation_system/src/Shaders/skin.vert", "D:/projects/animation_system/src/Shaders/lit.frag");
    mDiffuseTexture = new Texture("D:/projects/animation_system/assets/Woman.png");

    mGPUAnimInfo.animatedPose = mSkeleton.restPose;
    mGPUAnimInfo.posePalette.resize(mGPUAnimInfo.animatedPose.Size());

    mCPUAnimInfo.animatedPose = mSkeleton.restPose;
    mCPUAnimInfo.posePalette.resize(mCPUAnimInfo.animatedPose.Size());

    mGPUAnimInfo.model.position = vec3(-2, 0, 0);
    mCPUAnimInfo.model.position = vec3(2, 0, 0);

    for (unsigned int i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i].name == "Walking")
        {
            mCPUAnimInfo.clip = i;
        }
        else if (mClips[i].name == "Running")
        {
            mGPUAnimInfo.clip = i;
        }
    }
}

void SampleRenderer::Update(float inDeltaTime)
{
    mCPUAnimInfo.playback = mClips[mCPUAnimInfo.clip].Sample(mCPUAnimInfo.animatedPose, mCPUAnimInfo.playback + inDeltaTime);
    mGPUAnimInfo.playback = mClips[mGPUAnimInfo.clip].Sample(mGPUAnimInfo.animatedPose, mGPUAnimInfo.playback + inDeltaTime);

    for (auto& mesh : mCPUMeshes) {
        mesh.CPUSkin(mSkeleton, mCPUAnimInfo.animatedPose);
    }

    mGPUAnimInfo.animatedPose.GetMatrixPalette(mGPUAnimInfo.posePalette);
}

void SampleRenderer::Render(float inAspectRatio)
{
    mat4 projection = transposed(perspective(60.0f, inAspectRatio, 0.01f, 1000.0f));
    mat4 view = lookAt(vec3(0, 5, 7), vec3(0, 3, 0), vec3(0, 1, 0));
    mat4 model;

    // CPU Skinned Mesh
    model = MatrixFromTransform(mCPUAnimInfo.model);
    mStaticShader->Bind();
    uniform::Update<mat4>(mStaticShader->GetUniform("model"), model);
    uniform::Update<mat4>(mStaticShader->GetUniform("view"), view);
    uniform::Update<mat4>(mStaticShader->GetUniform("projection"), projection);
    uniform::Update<vec3>(mStaticShader->GetUniform("light"), vec3(1, 1, 1));

    mDiffuseTexture->Bind(mStaticShader->GetUniform("tex0"), 0);
    for (unsigned int i = 0, size = (unsigned int)mCPUMeshes.size(); i < size; ++i) {
        mCPUMeshes[i].Bind(mStaticShader->GetAttribute("position"), mStaticShader->GetAttribute("normal"), mStaticShader->GetAttribute("texCoord"), -1, -1);
        mCPUMeshes[i].Draw();
        mCPUMeshes[i].UnBind(mStaticShader->GetAttribute("position"), mStaticShader->GetAttribute("normal"), mStaticShader->GetAttribute("texCoord"), -1, -1);
    }
    mDiffuseTexture->UnBind(0);
    mStaticShader->UnBind();

    // GPU Skinned Mesh
    model = MatrixFromTransform(mGPUAnimInfo.model);
    mSkinnedShader->Bind();
    uniform::Update<mat4>(mSkinnedShader->GetUniform("model"), model);
    uniform::Update<mat4>(mSkinnedShader->GetUniform("view"), view);
    uniform::Update<mat4>(mSkinnedShader->GetUniform("projection"), projection);
    uniform::Update<vec3>(mSkinnedShader->GetUniform("light"), vec3(1, 1, 1));

    uniform::Update<mat4>(mSkinnedShader->GetUniform("pose"), mGPUAnimInfo.posePalette);
    uniform::Update<mat4>(mSkinnedShader->GetUniform("invBindPose"), mSkeleton.inverseBindPose);

    mDiffuseTexture->Bind(mSkinnedShader->GetUniform("tex0"), 0);
    for (unsigned int i = 0, size = (unsigned int)mGPUMeshes.size(); i < size; ++i) {
        mGPUMeshes[i].Bind(mSkinnedShader->GetAttribute("position"), mSkinnedShader->GetAttribute("normal"), mSkinnedShader->GetAttribute("texCoord"), mSkinnedShader->GetAttribute("weights"), mSkinnedShader->GetAttribute("joints"));
        mGPUMeshes[i].Draw();
        mGPUMeshes[i].UnBind(mSkinnedShader->GetAttribute("position"), mSkinnedShader->GetAttribute("normal"), mSkinnedShader->GetAttribute("texCoord"), mSkinnedShader->GetAttribute("weights"), mSkinnedShader->GetAttribute("joints"));
    }
    mDiffuseTexture->UnBind(0);
    mSkinnedShader->UnBind();
}

void SampleRenderer::Shutdown()
{
    delete mStaticShader;
    delete mDiffuseTexture;
    delete mSkinnedShader;
    mClips.clear();
    mCPUMeshes.clear();
    mGPUMeshes.clear();
}