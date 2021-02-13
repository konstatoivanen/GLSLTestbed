#include "PrecompiledHeader.h"
#include "Rendering/LightingUtility.h"
#include "Rendering/Graphics.h"

namespace LightingUtility
{
    void SetOEMTextures(const GraphicsID* atlases, uint32_t count, int probeSize)
    {
        float OEMRoughnessLevels[] = { 0.1f, 0.25f, 0.9f };

        PK_CORE_ASSERT(count == 3, "Tried to set an incorrect number (%i) of oem atlases. Supported count is %i", count, 3);

        Graphics::SetGlobalTexture(HashCache::pk_SceneOEM_HDR, atlases, 3);
        Graphics::SetGlobalFloat(HashCache::pk_SceneOEM_RVS, &OEMRoughnessLevels[0], 3);
        Graphics::SetGlobalFloat4(HashCache::pk_SceneOEM_ST, { 0, 0, 1, 1 });
    }
}