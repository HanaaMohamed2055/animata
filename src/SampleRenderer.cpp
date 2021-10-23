#include "SampleRenderer.h"
#include "Camera.h"

using namespace gfx;
using namespace math;

#define DEG2RAD 0.0174533f

void SampleRenderer::Initialize()
{
    mRotation = 45.0f;
    mShader = new Shader("D:/projects/animation_system/src/Shaders/static.vert", "D:/projects/animation_system/src/Shaders/static.frag");
    mDisplayTexture = new Texture("D:/projects/animation_system/src/Assets/uv.png");

    std::vector<vec3> positions;
    positions.push_back(vec3(-1, -1, 0));
    positions.push_back(vec3(-1, 1, 0));
    positions.push_back(vec3(1, -1, 0));
    positions.push_back(vec3(1, 1, 0));
    mVertexPositions = new VertexBuffer<vec3>(positions);

    std::vector<unsigned int> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(3);
    mIndexBuffer = new IndexBuffer(indices);

    std::vector<vec3> normals;
    normals.resize(4, vec3(0, 0, 1));
    mVertexNormals = new VertexBuffer<vec3>(normals);

    std::vector<vec2> uvs;
    uvs.push_back(vec2(0, 0));
    uvs.push_back(vec2(0, 1));
    uvs.push_back(vec2(1, 0));
    uvs.push_back(vec2(1, 1));
    mVertexTexCoords = new VertexBuffer<vec2>(uvs);
}

void SampleRenderer::Update(float inDeltaTime)
{
    mRotation += inDeltaTime * 45.0f;
    while (mRotation > 360.0f) {
        mRotation -= 360.0f;
    }
}

void SampleRenderer::Render(float inAspectRatio)
{
    mat4 projection = transposed(perspective(60.0f, inAspectRatio, 0.01f, 1000.0f));
    mat4 view = lookAt(vec3(0, 0, -5), vec3(0, 0, 0), vec3(0, 1, 0));
    mat4 model = MatrixFromQuaternion(quaternionFromAngleAxis(mRotation * DEG2RAD, vec3(0, 0, 1)));

    mShader->Bind();

    mVertexPositions->Bind(mShader->GetAttribute("position"));
    mVertexNormals->Bind(mShader->GetAttribute("normal"));
    mVertexTexCoords->Bind(mShader->GetAttribute("texCoord"));

    uniform::Update<mat4>(mShader->GetUniform("model"), model);
    uniform::Update<mat4>(mShader->GetUniform("view"), view);
    uniform::Update<mat4>(mShader->GetUniform("projection"), projection);

    uniform::Update<vec3>(mShader->GetUniform("light"), vec3(0, 0, 1));

    mDisplayTexture->Bind(mShader->GetUniform("tex0"), 0);

    draw::Draw(*mIndexBuffer, draw::DRAW_MODE::TRIANGLES);

    mDisplayTexture->UnBind(0);

    mVertexPositions->UnBind(mShader->GetAttribute("position"));
    mVertexNormals->UnBind(mShader->GetAttribute("normal"));
    mVertexTexCoords->UnBind(mShader->GetAttribute("texCoord"));

    mShader->unBind();
}

void SampleRenderer::Shutdown()
{
    delete mShader;
    delete mDisplayTexture;
    delete mVertexPositions;
    delete mVertexNormals;
    delete mVertexTexCoords;
    delete mIndexBuffer;
}