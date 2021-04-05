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
        const RenderTexture* ActiveRenderTarget;
        Ref<ShaderVariant> ActiveShader;
        Ref<Mesh> BlitQuad;
        Weak<Shader> BlitShader;
        uint32_t ViewPortX;
        uint32_t ViewPortY;
        uint32_t ViewPortW;
        uint32_t ViewPortH;
    };
}