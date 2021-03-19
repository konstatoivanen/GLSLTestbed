#pragma once
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

class FilterBase
{
    public:
        FilterBase(Weak<Shader> shader) { m_shader = shader; }
        virtual void Execute(Ref<RenderTexture> source, Ref<RenderTexture> destination) = 0;
    protected: 
        ShaderPropertyBlock m_properties;
        Weak<Shader> m_shader;
        virtual ~FilterBase() = 0 {}
};