#pragma once
#include "Rendering\ShaderPropertyBlock.h"
#include "Rendering\Shader.h"
#include "Rendering\Mesh.h"
#include "Rendering\RenderTexture.h"

struct GraphicsContext
{
    ShaderPropertyBlock ShaderProperties;
    Ref<RenderTexture> ActiveRenderTarget;
    Ref<Mesh> BlitQuad;
    Ref<Shader> BlitShader;
};