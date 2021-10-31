#pragma once
#include "Application.h"
#include "Gfx.h"
#include "Math.h"

class SampleRenderer : public Application
{
    gfx::Shader* mShader;
    gfx::VertexBuffer<math::vec3>* mVertexPositions;
    gfx::IndexBuffer* mIndexBuffer;
    gfx::VertexBuffer<math::vec3>* mVertexNormals;
    gfx::VertexBuffer<math::vec2>* mVertexTexCoords;
    gfx::Texture* mDisplayTexture;
    float mRotation;

public:
    void Initialize() override;
    void Update(float inDeltaTime) override;
    void Render(float inAspectRatio) override;
    void Shutdown() override;
};