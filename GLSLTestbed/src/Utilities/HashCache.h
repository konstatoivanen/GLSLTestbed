#pragma once
#include "Core/ISingleton.h"
#include "Core/IService.h"
#include "Utilities/StringHashID.h"

class HashCache : public IService, public ISingleton<HashCache>
{
    private: 
        #define DEFINE_HASH_CACHE(name) uint32_t name = StringHashID::StringToID(#name); \

    public:
        DEFINE_HASH_CACHE(_MainTex)
        DEFINE_HASH_CACHE(pk_Time)
        DEFINE_HASH_CACHE(pk_SinTime)
        DEFINE_HASH_CACHE(pk_CosTime)
        DEFINE_HASH_CACHE(pk_DeltaTime)
        DEFINE_HASH_CACHE(pk_WorldSpaceCameraPos)
        DEFINE_HASH_CACHE(pk_ProjectionParams)
        DEFINE_HASH_CACHE(pk_ScreenParams)
        DEFINE_HASH_CACHE(pk_ZBufferParams)
        DEFINE_HASH_CACHE(pk_MATRIX_M)
        DEFINE_HASH_CACHE(pk_MATRIX_I_M)
        DEFINE_HASH_CACHE(pk_MATRIX_V)
        DEFINE_HASH_CACHE(pk_MATRIX_P)
        DEFINE_HASH_CACHE(pk_MATRIX_VP)
        DEFINE_HASH_CACHE(pk_MATRIX_I_VP)
        uint32_t pk_SHA[3] = { StringHashID::StringToID("pk_SHAr"), StringHashID::StringToID("pk_SHAg"), StringHashID::StringToID("pk_SHAb") };
        uint32_t pk_SHB[3] = { StringHashID::StringToID("pk_SHBr"), StringHashID::StringToID("pk_SHBg"), StringHashID::StringToID("pk_SHBb") };
        DEFINE_HASH_CACHE(pk_SHC)
        DEFINE_HASH_CACHE(pk_SceneOEM_HDR)
        DEFINE_HASH_CACHE(pk_SceneOEM_ST)
        DEFINE_HASH_CACHE(pk_SceneOEM_RVS)
        DEFINE_HASH_CACHE(pk_Lights)
        DEFINE_HASH_CACHE(pk_LightCount)
        DEFINE_HASH_CACHE(pk_InstancingData)
        DEFINE_HASH_CACHE(pk_PerFrameConstants)
        DEFINE_HASH_CACHE(PK_ENABLE_INSTANCING)
};