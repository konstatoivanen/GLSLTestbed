#include "PrecompiledHeader.h"
#include "Utilities/HashCache.h"
#include "Rendering/LightingUtility.h"
#include "Rendering/Graphics.h"

namespace LightingUtility
{
    void SetOEMTextures(const GraphicsID texture, int probeSize)
    {
        float OEMRoughnessLevels[] = { 0.1f, 0.25f, 0.9f };

        auto* hashCache = HashCache::Get();

        Graphics::SetGlobalTexture(hashCache->pk_SceneOEM_HDR, texture);
        Graphics::SetGlobalFloat(hashCache->pk_SceneOEM_RVS, &OEMRoughnessLevels[0], 3);
        Graphics::SetGlobalFloat4(hashCache->pk_SceneOEM_ST, { 0, 0, 1, 1 });
    }
}