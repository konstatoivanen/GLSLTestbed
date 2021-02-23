#include "PreCompiledHeader.h"
#include "Utilities/StringHashID.h"
#include "Utilities/HashCache.h"

uint32_t HashCache::_MainTex;
uint32_t HashCache::pk_Time;
uint32_t HashCache::pk_SinTime;
uint32_t HashCache::pk_CosTime;
uint32_t HashCache::pk_DeltaTime;
uint32_t HashCache::pk_WorldSpaceCameraPos;
uint32_t HashCache::pk_ProjectionParams;
uint32_t HashCache::pk_ScreenParams;
uint32_t HashCache::pk_ZBufferParams;
uint32_t HashCache::pk_MATRIX_M;
uint32_t HashCache::pk_MATRIX_I_M;
uint32_t HashCache::pk_MATRIX_V;
uint32_t HashCache::pk_MATRIX_P;
uint32_t HashCache::pk_MATRIX_VP;
uint32_t HashCache::pk_MATRIX_I_VP;
uint32_t HashCache::pk_SHA[3];
uint32_t HashCache::pk_SHB[3];
uint32_t HashCache::pk_SHC;
uint32_t HashCache::pk_SceneOEM_HDR;
uint32_t HashCache::pk_SceneOEM_ST;
uint32_t HashCache::pk_SceneOEM_RVS;

void HashCache::Intitialize()
{
    _MainTex = StringHashID::StringToID("_MainTex");
    pk_Time = StringHashID::StringToID("pk_Time");
    pk_SinTime = StringHashID::StringToID("pk_SinTime");
    pk_CosTime = StringHashID::StringToID("pk_CosTime");
    pk_DeltaTime = StringHashID::StringToID("pk_DeltaTime");
    pk_WorldSpaceCameraPos = StringHashID::StringToID("pk_WorldSpaceCameraPos");
    pk_ProjectionParams = StringHashID::StringToID("pk_ProjectionParams");
    pk_ScreenParams = StringHashID::StringToID("pk_ScreenParams");
    pk_ZBufferParams = StringHashID::StringToID("pk_ZBufferParams");
    pk_MATRIX_M = StringHashID::StringToID("pk_MATRIX_M");
    pk_MATRIX_I_M = StringHashID::StringToID("pk_MATRIX_I_M");
    pk_MATRIX_V = StringHashID::StringToID("pk_MATRIX_V");
    pk_MATRIX_P = StringHashID::StringToID("pk_MATRIX_P");
    pk_MATRIX_VP = StringHashID::StringToID("pk_MATRIX_VP");
    pk_MATRIX_I_VP = StringHashID::StringToID("pk_MATRIX_I_VP");
    pk_SHA[0] = StringHashID::StringToID("pk_SHAr");
    pk_SHA[1] = StringHashID::StringToID("pk_SHAg");
    pk_SHA[2] = StringHashID::StringToID("pk_SHAb");
    pk_SHB[0] = StringHashID::StringToID("pk_SHBr");
    pk_SHB[1] = StringHashID::StringToID("pk_SHBg");
    pk_SHB[2] = StringHashID::StringToID("pk_SHBb");
    pk_SHC = StringHashID::StringToID("pk_SHAC");

    pk_SceneOEM_HDR = StringHashID::StringToID("pk_SceneOEM_HDR");
    pk_SceneOEM_ST = StringHashID::StringToID("pk_SceneOEM_ST");
    pk_SceneOEM_RVS = StringHashID::StringToID("pk_SceneOEM_RVS");
}