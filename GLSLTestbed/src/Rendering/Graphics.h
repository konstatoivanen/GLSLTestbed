#pragma once
#include "Utilities/Ref.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Structs/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace PK::Rendering::GraphicsAPI
{
	using namespace Utilities;
	using namespace Objects;
	using namespace Structs;

	void Initialize();
	void Terminate();

	void OpenContext(GraphicsContext* context);
	void CloseContext();

	void StartWindow();
	void EndWindow();

	uint2 GetWindowResolution(GLFWwindow* window);
	uint2 GetActiveWindowResolution();
	float4x4 GetActiveViewProjectionMatrix();
	const RenderTexture* GetActiveRenderTarget();
	const RenderTexture* GetBackBuffer();
	int GetActiveShaderProgramId();

	void ResetResourceBindings();
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
	void SetGlobalUInt(uint32_t hashId, const uint* values, uint32_t count = 1);
	void SetGlobalUInt2(uint32_t hashId, const uint2* values, uint32_t count = 1);
	void SetGlobalUInt3(uint32_t hashId, const uint3* values, uint32_t count = 1);
	void SetGlobalUInt4(uint32_t hashId, const uint4* values, uint32_t count = 1);
	void SetGlobalTexture(uint32_t hashId, const GraphicsID* textureIds, uint32_t count = 1);
	void SetGlobalConstantBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count = 1);
	void SetGlobalComputeBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count = 1);
	void SetGlobalResourceHandle(uint32_t hashId, const ulong* handleIds, uint32_t count = 1);

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
	void SetGlobalUInt(uint32_t hashId, uint value);
	void SetGlobalUInt2(uint32_t hashId, const uint2& value);
	void SetGlobalUInt3(uint32_t hashId, const uint3& value);
	void SetGlobalUInt4(uint32_t hashId, const uint4& value);
	void SetGlobalTexture(uint32_t hashId, GraphicsID textureId);
	void SetGlobalConstantBuffer(uint32_t hashId, GraphicsID bufferId);
	void SetGlobalComputeBuffer(uint32_t hashId, GraphicsID bufferId);
	void SetGlobalResourceHandle(uint32_t hashId, const ulong handleId);
	void SetGlobalKeyword(uint32_t hashId, bool value);

	void Clear(const float4& color, float depth, GLuint clearFlags);
	void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	void SetViewPorts(const uint32_t offset, const float4* viewports, const uint32_t count);
	void SetViewProjectionMatrices(const float4x4& view, const float4x4& projection);
	void SetModelMatrix(const float4x4& matrix);
	void SetModelMatrix(const float4x4& matrix, const float4x4& invMatrix);
	void SetRenderTarget(const RenderTexture* renderTexture, bool updateViewport = true);
	void SetRenderTarget(const RenderTexture* renderTexture, const uint firstViewport, const float4* viewports, const uint viewportCount);
	void SetRenderBuffer(const GraphicsID renderTarget, const RenderBuffer* renderBuffer, GLenum attachment);
	void SetPass(Shader* shader, uint32_t pass = 0);
	void SetVertexBuffer(const VertexBuffer* buffer);
	void SetIndexBuffer(const IndexBuffer* buffer);

	void BindTextures(ushort location, const GraphicsID* graphicsIds, ushort count);
	void BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsIds, ushort count);

	void Blit(Shader* shader);
	void Blit(Shader* shader, const ShaderPropertyBlock& propertyBlock);
	void Blit(const RenderTexture* destination, Shader* shader);
	void Blit(const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock);
	void Blit(const Texture* source, const RenderTexture* destination);
	void Blit(const Texture* source, const RenderTexture* destination, Shader* shader);
	void Blit(const Texture* source, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock);

	void Blit(const Material* material);
	void Blit(const Material* material, const ShaderPropertyBlock& propertyBlock);
	void Blit(const RenderTexture* destination, const Material* material);
	void Blit(const RenderTexture* destination, const Material* material, const ShaderPropertyBlock& propertyBlock);
	void Blit(const Texture* source, const RenderTexture* destination, const Material* material);
	void Blit(const Texture* source, const RenderTexture* destination, const Material* material, const ShaderPropertyBlock& propertyBlock);

	void BlitInstanced(uint offset, uint count, Shader* shader);
	void BlitInstanced(uint offset, uint count, Shader* shader, const ShaderPropertyBlock& propertyBlock);
	void BlitInstanced(uint offset, uint count, const RenderTexture* destination, Shader* shader);
	void BlitInstanced(uint offset, uint count, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock);
	void BlitInstanced(uint offset, uint count, const Texture* source, const RenderTexture* destination, Shader* shader);
	void BlitInstanced(uint offset, uint count, const Texture* source, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock);
	
	void CopyRenderTexture(const RenderTexture* source, const RenderTexture* destination, GLbitfield mask, GLenum filter);

	void DrawMesh(const Mesh* mesh, int submesh);
	void DrawMesh(const Mesh* mesh, int submesh, Shader* shader);
	void DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix);
	void DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix, const float4x4& invMatrix);
	void DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const ShaderPropertyBlock& propertyBlock);
	void DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock);

	void DrawMesh(const Mesh* mesh, int submesh, const Material* material);
	void DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix);
	void DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix, const float4x4& invMatrix);
	void DrawMesh(const Mesh* mesh, int submesh, const Material* material, const ShaderPropertyBlock& propertyBlock);
	void DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock);

	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count);
	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader);
	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader, const ShaderPropertyBlock& propertyBlock);
	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material);
	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material, const ShaderPropertyBlock& propertyBlock);

	void DrawProcedural(Shader* shader, GLenum topology, size_t offset, size_t count);

	void DispatchCompute(Shader* shader, uint3 threadGroupSize);
	void DispatchCompute(const Material* material, uint3 threadGroupSize);
	void DispatchCompute(Shader* shader, uint3 threadGroupSize, const ShaderPropertyBlock& propertyBlock);

	void DispatchComputeIndirect(Shader* shader, const GraphicsID& argumentsBuffer, uint offset);
	void DispatchComputeIndirect(const Material* material, const GraphicsID& argumentsBuffer, uint offset);
	void DispatchComputeIndirect(Shader* shader, const GraphicsID& argumentsBuffer, uint offset, const ShaderPropertyBlock& propertyBlock);
}