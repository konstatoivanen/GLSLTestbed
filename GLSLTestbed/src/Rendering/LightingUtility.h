#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Utilities/HashCache.h"
#include "Rendering/Texture.h"
#include "Rendering/ShaderPropertyBlock.h"

namespace LightingUtility
{
    void SetOEMTextures(const GraphicsID* atlases, uint32_t count, int probeSize);
}
