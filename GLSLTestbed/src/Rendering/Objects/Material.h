#pragma once
#include "Core/AssetDataBase.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

class Material : public ShaderPropertyBlock, public Asset
{
    friend void AssetImporters::Import(const std::string& filepath, Ref<Material>& material);

    public:
        Material() {}
        Material(Ref<Shader> shader) { m_shader = shader; }
        Weak<Shader> GetShader() const { return m_shader; }

    private:
        Weak<Shader> m_shader;
};

