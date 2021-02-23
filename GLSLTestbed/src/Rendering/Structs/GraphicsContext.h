#pragma once
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

struct GraphicsContext
{
    ShaderPropertyBlock ShaderProperties;
    Ref<ConstantBuffer> FrameConstants;
    Ref<RenderTexture> ActiveRenderTarget;
    Ref<Mesh> BlitQuad;
    Weak<Shader> BlitShader;
};