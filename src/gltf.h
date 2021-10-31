#pragma once
#include "cgltf.h"
#include "Animation.h"
#include "Skinning.h"

namespace gltf
{
    cgltf_data* LoadGLTFFile(const char* filePath);
    void FreeGLTFFile(cgltf_data*& handle);
    animation::Pose LoadRestPose(cgltf_data* data);
    animation::Pose LoadBindPose(cgltf_data* data);
    std::vector<std::string> LoadJointNames(cgltf_data* data);
    void LoadAnimationClips(std::vector<animation::Clip>& clips, cgltf_data* data);
    skin::Skeleton LoadSkeleton(cgltf_data* data);
    void LoadMeshes(std::vector<skin::AnimatedMesh>& meshes, cgltf_data* data);
}