#include "PrecompiledHeader.h"
#include "Utilities/HashCache.h"
#include "Rendering/LightingUtility.h"
#include "Rendering/Graphics.h"

namespace LightingUtility
{
    void SetOEMTextures(const GraphicsID texture, int probeSize)
    {
        float OEMRoughnessLevels[] = { 0.0f, 0.33f, 0.66f, 1.0f };

        auto* hashCache = HashCache::Get();

        Graphics::SetGlobalTexture(hashCache->pk_SceneOEM_HDR, texture);
        Graphics::SetGlobalFloat(hashCache->pk_SceneOEM_RVS, &OEMRoughnessLevels[0], 4);
        Graphics::SetGlobalFloat4(hashCache->pk_SceneOEM_ST, { 0, 0, 1, 1 });
    }
}