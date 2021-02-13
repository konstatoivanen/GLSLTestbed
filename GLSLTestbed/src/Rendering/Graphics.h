#pragma once
#include "PrecompiledHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <hlslmath.h>
#include "Utilities/Ref.h"
#include "Utilities/HashCache.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
#include "Rendering/RenderTexture.h"
#include "Rendering\GraphicsContext.h"

namespace Graphics
{
	void Initialize();
	void Terminate();

	void StartFrame(GraphicsContext* context);
	void EndFrame(GLFWwindow* window);
	int2 GetActiveResolution(GLFWwindow* window);
	Ref<RenderTexture>& GetActiveRenderTarget();
	Ref<RenderTexture> GetBackBuffer();
	int GetActiveShaderProgramId();

	void ClearGlobalProperties();
	void SetGlobalFloat(uint32_t hashId, const float* values, uint32_t count = 1);
	void SetGlobalFloat2(uint32_t hashId, const float2* values, uint32_t count = 1);
	void SetGlobalFloat3(uint32_t hashId, const float3* values, uint32_t count = 1);
	void SetGlobalFloat4(uint32_t hashId, const float4* values, uint32_t count = 1); 
	void SetGlobalFloat2x2(uint32_t hashId, const float2x2* values, uint32_t count = 1);
	void SetGlobalFloat3x3(uint32_t hashId, const float3x3* values, uint32_t count = 1);
	void SetGlobalFloat4x4(uint32_t hashId, const float4x4* values, uint32_t count = 1);
	void SetGlobalInt(uint32_t hashId, const int* values, uint32_t count = 1);
	void SetGlobalInt2(uint32_t hashId, const int2* values, uint32_t count = 1);
	void SetGlobalInt3(uint32_t hashId, const int3* values, uint32_t count = 1);
	void SetGlobalInt4(uint32_t hashId, const int4* values, uint32_t count = 1); 
	void SetGlobalTexture(uint32_t hashId, const GraphicsID* textureIds, uint32_t count = 1);

	void SetGlobalFloat(uint32_t hashId, float value);
	void SetGlobalFloat2(uint32_t hashId, const float2& value);
	void SetGlobalFloat3(uint32_t hashId, const float3& value); 
	void SetGlobalFloat4(uint32_t hashId, const float4& value); 
	void SetGlobalFloat2x2(uint32_t hashId, const float2x2& value);
	void SetGlobalFloat3x3(uint32_t hashId, const float3x3& value);
	void SetGlobalFloat4x4(uint32_t hashId, const float4x4& value);
	void SetGlobalInt(uint32_t hashId, int value);
	void SetGlobalInt2(uint32_t hashId, const int2& value);
	void SetGlobalInt3(uint32_t hashId, const int3& value);
	void SetGlobalInt4(uint32_t hashId, const int4& value);
	void SetGlobalTexture(uint32_t hashId, GraphicsID textureId);

	void Clear(const float4& color, float depth, GLuint clearFlags);
	void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	void SetViewProjectionMatrices(const float4x4& view, const float4x4& projection);
	void SetModelMatrix(const float4x4& matrix);
	void SetRenderTarget(const Ref<RenderTexture>& renderTexture);
	void SetRenderBuffer(const Ref<RenderBuffer>& renderBuffer, GLenum attachment);
	void SetPass(const Ref<Shader>& shader, uint32_t pass = 0);
	void SetMesh(const Ref<Mesh>& mesh);

	void Blit(const Ref<Shader>& shader);
	void Blit(const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock);
	void Blit(const Ref<RenderTexture>& destination, const Ref<Shader>& shader);
	void Blit(const Ref<RenderTexture>& destination, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock);
	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination);
	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination, const Ref<Shader>& shader);
	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock);

	void DrawMesh(const Ref<Mesh>& mesh);
	void DrawMesh(const Ref<Mesh>& mesh, const Ref<Shader>& shader);
	void DrawMesh(const Ref<Mesh>& mesh, const Ref<Shader>& shader, const float4x4& matrix);
	void DrawMesh(const Ref<Mesh>& mesh, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock);
	void DrawMesh(const Ref<Mesh>& mesh, const Ref<Shader>& shader, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock);
}