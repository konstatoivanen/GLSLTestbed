#pragma once
#ifndef PK_INSTANCING
#define PK_INSTANCING

#include HLSLSupport.glsl

#if defined(PK_ENABLE_INSTANCING)
    PK_DECLARE_READONLY_BUFFER(uint, pk_InstancingPropertyIndices);
    
    #if defined(SHADER_STAGE_VERTEX)

        #define PK_VARYING_INSTANCE_ID out flat uint3 pk_instanceIds;
        #define PK_INSTANCE_BASE_ID gl_InstanceID
        #define PK_INSTANCE_OFFSET_ID (PK_INSTANCE_BASE_ID + gl_BaseInstance)
        #define PK_INSTANCE_PROPERTIES_ID PK_BUFFER_DATA(pk_InstancingPropertyIndices, PK_INSTANCE_OFFSET_ID)
        #define PK_SETUP_INSTANCE_ID() pk_instanceIds = uint3(PK_INSTANCE_BASE_ID, PK_INSTANCE_OFFSET_ID, PK_INSTANCE_PROPERTIES_ID)

    #elif defined(SHADER_STAGE_GEOMETRY)

        out flat uint3 pk_instanceIds;
        #define PK_VARYING_STREAM_INSTANCE_ID uint3 pk_instanceIds;
        #define PK_STREAM_INSTANCE_IDS gl_in[0].pk_instanceIds;
        #define PK_INSTANCE_BASE_ID pk_instanceIds[0]
        #define PK_INSTANCE_OFFSET_ID pk_instanceIds[1]
        #define PK_INSTANCE_PROPERTIES_ID pk_instanceIds[2]
        #define PK_SETUP_INSTANCE_ID() pk_instanceIds = PK_STREAM_INSTANCE_IDS

    #elif defined(SHADER_STAGE_FRAGMENT)

        uint3 pk_LocalInstanceIds;
        #define PK_VARYING_INSTANCE_ID in flat uint3 pk_instanceIds;
        #define PK_INSTANCE_BASE_ID pk_LocalInstanceIds[0]
        #define PK_INSTANCE_OFFSET_ID pk_LocalInstanceIds[1]
        #define PK_INSTANCE_PROPERTIES_ID pk_LocalInstanceIds[2]
        #define PK_SETUP_INSTANCE_ID() pk_LocalInstanceIds = pk_instanceIds

    #endif

    #define PK_INSTANCED_PROPERTY 
    #define PK_ACCESS_INSTANCED_PROP(Name) PK_BUFFER_DATA(pk_InstancedProperties, PK_INSTANCE_PROPERTIES_ID).Name

    #define PK_BEGIN_INSTANCED_PROPERTIES \
        struct PkInstancedProperties      \
        {                                 \
        
    #define PK_END_INSTANCED_PROPERTIES                                            \
        };                                                                         \
        PK_DECLARE_READONLY_BUFFER(PkInstancedProperties, pk_InstancedProperties); \

#else
    #define PK_VARYING_INSTANCE_ID
    #define PK_INSTANCED_PROPERTY uniform 
    #define PK_INSTANCE_ID 0
    #define PK_SETUP_INSTANCE_ID()
    #define PK_ACCESS_INSTANCED_PROP(Name) Name
    #define PK_BEGIN_INSTANCED_PROPERTIES
    #define PK_END_INSTANCED_PROPERTIES
#endif

#endif