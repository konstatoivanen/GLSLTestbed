#pragma once
#include "Utilities/Ref.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Structs/GraphicsContext.h"
#include <GLFW/glfw3.h>
#include <hlslmath.h>

namespace Graphics
{
	void Initialize();
	void Terminate();

	void OpenContext(GraphicsContext* context);
	void CloseContext();

	void StartWindow();
	void EndWindow();

	int2 GetWindowResolution(GLFWwindow* window);
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
	void SetGlobalConstantBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count = 1);

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
	void SetGlobalConstantBuffer(uint32_t hashId, GraphicsID bufferId);
	void SetGlobalKeyword(uint32_t hashId, bool value);

	void Clear(const float4& color, float depth, GLuint clearFlags);
	void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	void SetViewProjectionMatrices(const float4x4& view, const float4x4& projection);
	void SetModelMatrix(const float4x4& matrix);
	void SetRenderTarget(const Ref<RenderTexture>& renderTexture);
	void SetRenderBuffer(const Ref<RenderBuffer>& renderBuffer, GLenum attachment);
	void SetPass(const Ref<Shader>& shader, uint32_t pass = 0);
	void SetMesh(const Ref<Mesh>& mesh);
	void SetVertexBuffer(const Ref<VertexBuffer>& buffer);
	void SetIndexBuffer(const Ref<IndexBuffer>& buffer);

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