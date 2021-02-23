#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/GraphicsObject.h"
#include "Rendering/Structs/PropertyBlock.h"
#include <hlslmath.h>
#include <ext.hpp>

class ShaderPropertyBlock : public PropertyBlock
{
    public:
		void SetTexture(uint32_t hashId, const GraphicsID* textureIds, uint32_t count = 1) { SetValue(hashId, CG_TYPE_TEXTURE, textureIds, count); }
		void SetConstantBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count = 1) { SetValue(hashId, CG_TYPE_CONSTANT_BUFFER, bufferIds, count); }
		void SetTexture(uint32_t hashId, GraphicsID textureId) { SetValue(hashId, CG_TYPE_TEXTURE, &textureId); }
		void SetConstantBuffer(uint32_t hashId, GraphicsID bufferId) { SetValue(hashId, CG_TYPE_CONSTANT_BUFFER, &bufferId); }
		void SetKeyword(uint32_t hashId, bool value);
        void Clear() override;
		const std::vector<uint32_t>& GetKeywords() const { return m_keywords; }
    private:
		std::vector<uint32_t> m_keywords;
};
