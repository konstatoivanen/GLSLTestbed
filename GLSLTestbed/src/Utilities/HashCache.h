#pragma once
#include <cstdint>

class HashCache
{
    public:
        static void Intitialize();

        static uint32_t _MainTex;
        static uint32_t pk_Time;
        static uint32_t pk_SinTime;
        static uint32_t pk_CosTime;
        static uint32_t pk_DeltaTime;
        static uint32_t pk_WorldSpaceCameraPos;
        static uint32_t pk_ProjectionParams;
        static uint32_t pk_ScreenParams;
        static uint32_t pk_ZBufferParams;
        static uint32_t pk_MATRIX_M;
        static uint32_t pk_MATRIX_I_M;
        static uint32_t pk_MATRIX_V;
        static uint32_t pk_MATRIX_P;
        static uint32_t pk_MATRIX_VP;
        static uint32_t pk_MATRIX_I_VP;
        static uint32_t pk_SHA[3];
        static uint32_t pk_SHB[3];
        static uint32_t pk_SHC;
        static uint32_t pk_SceneOEM_HDR;
        static uint32_t pk_SceneOEM_ST;
        static uint32_t pk_SceneOEM_RVS;
};