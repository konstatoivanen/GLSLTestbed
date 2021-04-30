#pragma once
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterBase
    {
        public:
            FilterBase(Shader* shader) { m_shader = shader; }
            virtual void OnPreRender(const RenderTexture* source) = 0;
            virtual void Execute(const RenderTexture* source, const RenderTexture* destination) = 0;
        protected: 
            Structs::ShaderPropertyBlock m_properties;
            Shader* m_shader = nullptr;
            virtual ~FilterBase() = default;
    };
}