#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Utilities/Ref.h"
#include "Utilities/HashCache.h"
#include "Rendering/Objects/Texture2D.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

namespace LightingUtility
{
    void SetOEMTextures(const GraphicsID* atlases, uint32_t count, int probeSize);
    void SetOEMTextures(const Weak<Texture2D>* atlases, uint32_t count, int probeSize);
}
