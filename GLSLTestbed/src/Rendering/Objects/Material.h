#pragma once
#include "Core/AssetDataBase.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

namespace PK::Rendering::Objects
{
    using namespace Utilities;
    using namespace Structs;

    class Material : public ShaderPropertyBlock, public Asset
    {
        friend void AssetImporters::Import(const std::string& filepath, Ref<Material>& material);
    
        public:
            Material() {}
            Material(Ref<Shader> shader) { m_shader = shader; }
            Weak<Shader> GetShader() const { return m_shader; }
            const uint32_t GetRenderQueueIndex() const { return m_renderQueueIndex; }

        private:
            uint32_t m_renderQueueIndex;
            Weak<Shader> m_shader;
    };
}