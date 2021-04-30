#pragma once
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"
#include "Rendering/Structs/FixedStateAttributes.h"
#include "Rendering/Structs/ResourceBindState.h"

namespace PK::Rendering::Structs
{
    using namespace Utilities;
    using namespace Objects;

    struct GraphicsContext
    {
        ShaderPropertyBlock ShaderProperties;
        FixedStateAttributes FixedStateAttributes;
        ResourceBindState ResourceBindState;
        const RenderTexture* ActiveRenderTarget = nullptr;
        Ref<ShaderVariant> ActiveShader;
        Ref<Mesh> BlitQuad;
        Shader* BlitShader = nullptr;
        uint4 ViewPort = CG_UINT4_ZERO;
        color ClearColor = CG_COLOR_CLEAR;
        float ClearDepth = 0.0f;
    };
}