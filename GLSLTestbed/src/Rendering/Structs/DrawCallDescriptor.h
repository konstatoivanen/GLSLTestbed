#pragma once
#include <Rendering/Objects/Mesh.h>
#include <Rendering/Objects/Shader.h>
#include <Rendering/Objects/RenderTexture.h>

namespace PK::Rendering::Structs
{
    using namespace PK::Rendering::Objects;

    enum class DrawCommand : uint
    {
        Mesh = 0,
        MeshInstanced = 1,
        Procedural = 2,
        Compute = 3,
        ComputeIndirect = 4,
    };

    struct DrawCallDescriptor
    {
        const Mesh* mesh = nullptr;
        Shader* shader = nullptr;
        const Texture* source = nullptr;
        const RenderTexture* destination = nullptr;
        const ShaderPropertyBlock* propertyBlock0 = nullptr;
        const ShaderPropertyBlock* propertyBlock1 = nullptr; 
        const FixedStateAttributes* attributes = nullptr;
        const float4x4* matrix = nullptr;
        const float4x4* invMatrix = nullptr;
        GraphicsID argumentsBufferId = 0;

        DrawCommand command = DrawCommand::Mesh;
        int submesh = -1;
        size_t offset = 0;
        size_t count = 0;
        uint3 threadGroupSize = PK_UINT3_ZERO;
        GLenum topology = GL_TRIANGLES;
        GLenum memoryBarrierFlags = 0;
    };
}