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

        DEFINE_HASH_CACHE(pk_Lights)
        DEFINE_HASH_CACHE(pk_LightCount)
        DEFINE_HASH_CACHE(pk_InstancingMatrices)
        DEFINE_HASH_CACHE(pk_PerFrameConstants)
        DEFINE_HASH_CACHE(PK_ENABLE_INSTANCING)
        DEFINE_HASH_CACHE(pk_GizmoVertices)
        DEFINE_HASH_CACHE(_Bloom_Intensity)
        DEFINE_HASH_CACHE(_Bloom_LensDirtIntensity)
        DEFINE_HASH_CACHE(_Bloom_DirtTexture)
        DEFINE_HASH_CACHE(_Bloom_BlurSize)
        DEFINE_HASH_CACHE(_Bloom_Textures)
        DEFINE_HASH_CACHE(_Tonemap_Exposure)
        
        DEFINE_HASH_CACHE(_Intensity)
        DEFINE_HASH_CACHE(_Radius)
        DEFINE_HASH_CACHE(_TargetScale)
        DEFINE_HASH_CACHE(_BlurVector)

        DEFINE_HASH_CACHE(pk_ScreenDepth)
        DEFINE_HASH_CACHE(pk_ScreenNormals)
        DEFINE_HASH_CACHE(pk_ScreenOcclusion)

        DEFINE_HASH_CACHE(pk_FrustumTileSizes)
        DEFINE_HASH_CACHE(pk_FrustumTileScaleBias)
        DEFINE_HASH_CACHE(pk_GlobalLightsList)
        DEFINE_HASH_CACHE(pk_LightTiles)
        DEFINE_HASH_CACHE(pk_FDepthRanges)
        DEFINE_HASH_CACHE(pk_ClusterDispatchInfo)
        DEFINE_HASH_CACHE(pk_VisibleClusters)

        #undef DEFINE_HASH_CACHE
    };
}