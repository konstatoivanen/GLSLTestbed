#pragma once
#include "Core/ISingleton.h"
#include "Core/IService.h"
#include "Utilities/StringHashID.h"

namespace PK::Utilities
{
    struct HashCache : public IService, public ISingleton<HashCache>
    {
        #define DEFINE_HASH_CACHE(name) uint32_t name = StringHashID::StringToID(#name); \
    
        DEFINE_HASH_CACHE(_MainTex)
        DEFINE_HASH_CACHE(pk_Time)
        DEFINE_HASH_CACHE(pk_SinTime)
        DEFINE_HASH_CACHE(pk_CosTime)
        DEFINE_HASH_CACHE(pk_DeltaTime)
        DEFINE_HASH_CACHE(pk_WorldSpaceCameraPos)
        DEFINE_HASH_CACHE(pk_ProjectionParams)
        DEFINE_HASH_CACHE(pk_ExpProjectionParams)
        DEFINE_HASH_CACHE(pk_ScreenParams)
        DEFINE_HASH_CACHE(pk_MATRIX_M)
        DEFINE_HASH_CACHE(pk_MATRIX_I_M)
        DEFINE_HASH_CACHE(pk_MATRIX_V)
        DEFINE_HASH_CACHE(pk_MATRIX_I_V)
        DEFINE_HASH_CACHE(pk_MATRIX_P)
        DEFINE_HASH_CACHE(pk_MATRIX_I_P)
        DEFINE_HASH_CACHE(pk_MATRIX_VP)
        DEFINE_HASH_CACHE(pk_MATRIX_I_VP)
        uint32_t pk_SHA[3] = { StringHashID::StringToID("pk_SHAr"), StringHashID::StringToID("pk_SHAg"), StringHashID::StringToID("pk_SHAb") };
        uint32_t pk_SHB[3] = { StringHashID::StringToID("pk_SHBr"), StringHashID::StringToID("pk_SHBg"), StringHashID::StringToID("pk_SHBb") };
        DEFINE_HASH_CACHE(pk_SHC)
        
        DEFINE_HASH_CACHE(pk_SceneOEM_HDR)
        DEFINE_HASH_CACHE(pk_SceneOEM_ST)
        DEFINE_HASH_CACHE(pk_SceneOEM_RVS)
        DEFINE_HASH_CACHE(pk_SceneOEM_Exposure)

        DEFINE_HASH_CACHE(pk_InstancingMatrices)
        DEFINE_HASH_CACHE(pk_InstancingPropertyIndices)
        DEFINE_HASH_CACHE(pk_InstancedProperties)
        DEFINE_HASH_CACHE(PK_ENABLE_INSTANCING)

        DEFINE_HASH_CACHE(pk_PerFrameConstants)
        DEFINE_HASH_CACHE(pk_GizmoVertices)
        
        DEFINE_HASH_CACHE(_BloomPassParams)
        DEFINE_HASH_CACHE(_AOPassParams)
        DEFINE_HASH_CACHE(_AOParams)

        DEFINE_HASH_CACHE(pk_ScreenDepth)
        DEFINE_HASH_CACHE(pk_ScreenNormals)
        DEFINE_HASH_CACHE(pk_ScreenOcclusion)
        DEFINE_HASH_CACHE(pk_ShadowmapAtlas)
        DEFINE_HASH_CACHE(pk_LightCookies)
        DEFINE_HASH_CACHE(pk_Bluenoise256)

        DEFINE_HASH_CACHE(pk_Lights)
        DEFINE_HASH_CACHE(pk_LightMatrices)
        DEFINE_HASH_CACHE(pk_LightDirections)
        DEFINE_HASH_CACHE(pk_LightCount)
        DEFINE_HASH_CACHE(pk_ClusterSizePx)
        DEFINE_HASH_CACHE(pk_GlobalLightsList)
        DEFINE_HASH_CACHE(pk_LightTiles)
        DEFINE_HASH_CACHE(pk_TileMaxDepths)
        DEFINE_HASH_CACHE(pk_GlobalListListIndex)

        #undef DEFINE_HASH_CACHE
    };
}