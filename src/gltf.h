#pragma once
#include "cgltf.h"

namespace gltf
{
    cgltf_data* LoadGLTFFile(const char* filePath);
    void FreeGLTFFile(cgltf_data*& handle);
}