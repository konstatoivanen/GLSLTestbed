#include "PrecompiledHeader.h"
#include "hlslmath.h"

ushort CGConvert::Size(ushort type)
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
		case CG_TYPE_COMPUTE_BUFFER: return CG_TYPE_SIZE_COMPUTE_BUFFER;
	}

	return CG_TYPE_ERROR;
}

ushort CGConvert::Components(ushort type)
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
		case CG_TYPE_COMPUTE_BUFFER: return CG_TYPE_COMPONENTS_COMPUTE_BUFFER;
	}

	return CG_TYPE_ERROR;
}

ushort CGConvert::BaseType(ushort type)
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
		case CG_TYPE_COMPUTE_BUFFER: return GL_INT;
	}

	return CG_TYPE_ERROR;
}

std::string CGConvert::ToString(ushort type)
{
	switch (type)
	{
		case CG_TYPE_FLOAT: return "FLOAT";
		case CG_TYPE_FLOAT2: return "FLOAT2";
		case CG_TYPE_FLOAT3: return "FLOAT3";
		case CG_TYPE_FLOAT4: return "FLOAT4";
		case CG_TYPE_FLOAT2X2: return "FLOAT2X2";
		case CG_TYPE_FLOAT3X3: return "FLOAT3X3";
		case CG_TYPE_FLOAT4X4: return "FLOAT4X4";
		case CG_TYPE_INT: return "INT";
		case CG_TYPE_INT2: return "INT2";
		case CG_TYPE_INT3: return "INT3";
		case CG_TYPE_INT4: return "INT4";
		case CG_TYPE_TEXTURE: return "TEXTURE";
		case CG_TYPE_CONSTANT_BUFFER: return "CONSTANT_BUFFER";
		case CG_TYPE_COMPUTE_BUFFER: return "COMPUTE_BUFFER";
	}

	return "INVALID";
}

ushort CGConvert::FromString(const char* string)
{
	if (strcmp(string, "FLOAT") == 0) return CG_TYPE_FLOAT;
	if (strcmp(string, "FLOAT2") == 0) return CG_TYPE_FLOAT2;
	if (strcmp(string, "FLOAT3") == 0) return CG_TYPE_FLOAT3;
	if (strcmp(string, "FLOAT4") == 0) return CG_TYPE_FLOAT4;
	if (strcmp(string, "FLOAT2X2") == 0) return CG_TYPE_FLOAT2X2;
	if (strcmp(string, "FLOAT3X3") == 0) return CG_TYPE_FLOAT3X3;
	if (strcmp(string, "FLOAT4X4") == 0) return CG_TYPE_FLOAT4X4;
	if (strcmp(string, "INT") == 0) return CG_TYPE_INT;
	if (strcmp(string, "INT2") == 0) return CG_TYPE_INT2;
	if (strcmp(string, "INT3") == 0) return CG_TYPE_INT3;
	if (strcmp(string, "INT4") == 0) return CG_TYPE_INT4;
	if (strcmp(string, "TEXTURE") == 0) return CG_TYPE_TEXTURE;
	if (strcmp(string, "CONSTANT_BUFFER") == 0) return CG_TYPE_CONSTANT_BUFFER;
	if (strcmp(string, "COMPUTE_BUFFER") == 0) return CG_TYPE_COMPUTE_BUFFER;
	return CG_TYPE_ERROR;
}
