#pragma once
#ifndef PK_INSTANCING
#define PK_INSTANCING

#include HLSLSupport.glsl

#if defined(PK_ENABLE_INSTANCING)
    uniform uint pk_InstancingOffset;

    
    #if defined(SHADER_STAGE_VERTEX)
        #define PK_VARYING_INSTANCE_ID out flat uint pk_instanceId;
        #define PK_INSTANCE_ID (gl_InstanceID + pk_InstancingOffset)
        #define PK_SETUP_INSTANCE_ID() pk_instanceId = PK_INSTANCE_ID
    #elif defined(SHADER_STAGE_FRAGMENT)
        uint pk_FragmentInstanceId;
        #define PK_VARYING_INSTANCE_ID in flat uint pk_instanceId;
        #define PK_INSTANCE_ID pk_FragmentInstanceId
        #define PK_SETUP_INSTANCE_ID() pk_FragmentInstanceId = pk_instanceId
    #endif

    #define PK_INSTANCED_PROPERTY 
    #define PK_ACCESS_INSTANCED_PROP(Name) PK_BUFFER_DATA(pk_InstancedProperties, PK_INSTANCE_ID).Name
#else
    #define PK_VARYING_INSTANCE_ID
    #define PK_INSTANCED_PROPERTY uniform 
    #define PK_INSTANCE_ID 0
    #define PK_SETUP_INSTANCE_ID()
    #define PK_ACCESS_INSTANCED_PROP(Name) Name
#endif


#define PK_BEGIN_INSTANCED_PROPERTIES \
    #if defined(PK_ENABLE_INSTANCING) \
    struct PkInstancedProperties      \
    {                                 \
    #endif                            \
    

#define PK_END_INSTANCED_PROPERTIES                                            \
    #if defined(PK_ENABLE_INSTANCING)                                          \
    };                                                                         \
    PK_DECLARE_READONLY_BUFFER(PkInstancedProperties, pk_InstancedProperties); \
    #endif                                                                     \

#endif