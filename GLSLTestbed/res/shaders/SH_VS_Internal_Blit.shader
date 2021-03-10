#version 460

#include includes/HLSLSupport.glsl

#pragma PROGRAM_VERTEX
#define USE_VERTEX_PROGRAM_IMG
#include includes/BlitCommon.glsl

#pragma PROGRAM_FRAGMENT
#define INCLUDE_FRAGMENT_PROGRAM_IMG
#include includes/BlitCommon.glsl