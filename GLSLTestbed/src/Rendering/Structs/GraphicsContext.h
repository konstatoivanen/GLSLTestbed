#pragma once
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"
#include "Rendering/Structs/FixedStateAttributes.h"

struct GraphicsContext
{
    ShaderPropertyBlock ShaderProperties;
    FixedStateAttributes FixedStateAttributes;
    Ref<RenderTexture> ActiveRenderTarget;
    Ref<ShaderVariant> ActiveShader;
    Ref<Mesh> BlitQuad;
    Weak<Shader> BlitShader;
};