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
            Material(Ref<Shader> shader) { m_shader = shader; m_cachedShaderAssetId = shader->GetAssetID(); }
            Weak<Shader> GetShader() const { return m_shader; }
            AssetID GetShaderAssetID() const { return m_cachedShaderAssetId; }
            const uint32_t GetRenderQueueIndex() const { return m_renderQueueIndex; }
            const bool SupportsInstancing() const { return m_shader.lock()->GetInstancingInfo().supportsInstancing; }

        private:
            std::vector<char> m_cachedInstancedProperties;
            uint32_t m_renderQueueIndex = 0;
            AssetID m_cachedShaderAssetId = 0;
            Weak<Shader> m_shader;
    };
}