#include "PrecompiledHeader.h"
#include "hlslmath.h"

ushort CGType::Size(ushort type)
{
	switch (type)
	{
		case CG_TYPE_FLOAT: return CG_TYPE_SIZE_FLOAT;
		case CG_TYPE_FLOAT2: return CG_TYPE_SIZE_FLOAT2;
		case CG_TYPE_FLOAT3: return CG_TYPE_SIZE_FLOAT3;
		case CG_TYPE_FLOAT4: return CG_TYPE_SIZE_FLOAT4;
		case CG_TYPE_FLOAT2X2: return CG_TYPE_SIZE_FLOAT2X2;
		case CG_TYPE_FLOAT3X3: return CG_TYPE_SIZE_FLOAT3X3;
		case CG_TYPE_FLOAT4X4: return CG_TYPE_SIZE_FLOAT4X4;
		case CG_TYPE_INT: return CG_TYPE_SIZE_INT;
		case CG_TYPE_INT2: return CG_TYPE_SIZE_INT2;
		case CG_TYPE_INT3: return CG_TYPE_SIZE_INT3;
		case CG_TYPE_INT4: return CG_TYPE_SIZE_INT4;
		case CG_TYPE_TEXTURE: return CG_TYPE_SIZE_TEXTURE;
		case CG_TYPE_CONSTANT_BUFFER: return CG_TYPE_SIZE_CONSTANT_BUFFER;
	}

	return CG_TYPE_ERROR;
}

ushort CGType::Components(ushort type)
{
	switch (type)
	{
		case CG_TYPE_FLOAT: return CG_TYPE_COMPONENTS_FLOAT;
		case CG_TYPE_FLOAT2: return CG_TYPE_COMPONENTS_FLOAT2;
		case CG_TYPE_FLOAT3: return CG_TYPE_COMPONENTS_FLOAT3;
		case CG_TYPE_FLOAT4: return CG_TYPE_COMPONENTS_FLOAT4;
		case CG_TYPE_FLOAT2X2: return CG_TYPE_COMPONENTS_FLOAT2X2;
		case CG_TYPE_FLOAT3X3: return CG_TYPE_COMPONENTS_FLOAT3X3;
		case CG_TYPE_FLOAT4X4: return CG_TYPE_COMPONENTS_FLOAT4X4;
		case CG_TYPE_INT: return CG_TYPE_COMPONENTS_INT;
		case CG_TYPE_INT2: return CG_TYPE_COMPONENTS_INT2;
		case CG_TYPE_INT3: return CG_TYPE_COMPONENTS_INT3;
		case CG_TYPE_INT4: return CG_TYPE_COMPONENTS_INT4;
		case CG_TYPE_TEXTURE: return CG_TYPE_COMPONENTS_TEXTURE;
		case CG_TYPE_CONSTANT_BUFFER: return CG_TYPE_COMPONENTS_CONSTANT_BUFFER;
	}

	return CG_TYPE_ERROR;
}

ushort CGType::BaseType(ushort type)
{
	switch (type)
	{
		case CG_TYPE_FLOAT: return GL_FLOAT;
		case CG_TYPE_FLOAT2: return GL_FLOAT;
		case CG_TYPE_FLOAT3: return GL_FLOAT;
		case CG_TYPE_FLOAT4: return GL_FLOAT;
		case CG_TYPE_FLOAT2X2: return GL_FLOAT;
		case CG_TYPE_FLOAT3X3: return GL_FLOAT;
		case CG_TYPE_FLOAT4X4: return GL_FLOAT;
		case CG_TYPE_INT: return GL_INT;
		case CG_TYPE_INT2: return GL_INT;
		case CG_TYPE_INT3: return GL_INT;
		case CG_TYPE_INT4: return GL_INT;
		case CG_TYPE_TEXTURE: return GL_INT;
		case CG_TYPE_CONSTANT_BUFFER: return GL_INT;
	}

	return CG_TYPE_ERROR;
}