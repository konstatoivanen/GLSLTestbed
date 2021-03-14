#pragma once
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"
#include "Rendering/Structs/FixedStateAttributes.h"
#include "Rendering/Structs/ResourceBindState.h"

struct GraphicsContext
{
    ShaderPropertyBlock ShaderProperties;
    FixedStateAttributes FixedStateAttributes;
    ResourceBindState ResourceBindState;
    Ref<RenderTexture> ActiveRenderTarget;
    Ref<ShaderVariant> ActiveShader;
    Ref<Mesh> BlitQuad;
    Weak<Shader> BlitShader;
};