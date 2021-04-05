#include "PrecompiledHeader.h"
#include "Utilities/HashCache.h"
#include "Rendering/Graphics.h"
#include <hlslmath.h>

namespace PK::Rendering::GraphicsAPI
{
	static GraphicsContext* m_currentContext;
	
	static inline GraphicsContext* GetCurrentContext()
	{
		PK_CORE_ASSERT(m_currentContext != nullptr, "Not In A Valid Graphics Context!")
		return m_currentContext;
	}

	#define CURRENT_WINDOW glfwGetCurrentContext()
	#define CURRENT_ATTRIBUTES GetCurrentContext()->FixedStateAttributes
	#define GLOBAL_KEYWORDS GetCurrentContext()->ShaderProperties.GetKeywords()
	#define GLOBAL_PROPERTIES GetCurrentContext()->ShaderProperties
	#define RESOURCE_BINDINGS GetCurrentContext()->ResourceBindState
	#define ACTIVE_RENDERTARGET GetCurrentContext()->ActiveRenderTarget
	#define BLIT_QUAD GetCurrentContext()->BlitQuad.get()
	#define BLIT_SHADER GetCurrentContext()->BlitShader.lock().get()
	#define DELTA_CHECK_SET(attrib, value, func) \
	{											 \
		auto& current = CURRENT_ATTRIBUTES;		 \
		if (current.value != attrib.value)		 \
		{										 \
			current.value = attrib.value;		 \
			func;								 \
		}										 \
	}											 \

	static inline void glToggle(GLenum enumKey, bool value)
	{
		if (value)
		{
			glEnable(enumKey);
		}
		else
		{
			glDisable(enumKey);
		}
	}

	static void SetFixedStateAttributes(const FixedStateAttributes& attributes)
	{
		DELTA_CHECK_SET(attributes, ZTestEnabled, glToggle(GL_DEPTH_TEST, attributes.ZTestEnabled))
		DELTA_CHECK_SET(attributes, BlendEnabled, glToggle(GL_BLEND, attributes.BlendEnabled))
		DELTA_CHECK_SET(attributes, CullEnabled, glToggle(GL_CULL_FACE, attributes.CullEnabled))
		DELTA_CHECK_SET(attributes, ZWriteEnabled, glDepthMask(attributes.ZWriteEnabled))
		DELTA_CHECK_SET(attributes, ZTest, if (attributes.ZTestEnabled) glDepthFunc(attributes.ZTest))
		DELTA_CHECK_SET(attributes, Blend, if (attributes.BlendEnabled) glBlendFunc(attributes.Blend.Source, attributes.Blend.Destination))
		DELTA_CHECK_SET(attributes, ColorMask, glColorMask(attributes.ColorMask & (1 << 0), attributes.ColorMask & (1 << 1), attributes.ColorMask & (1 << 2), attributes.ColorMask & (1 << 3)))
		DELTA_CHECK_SET(attributes, CullMode, if (attributes.CullEnabled) glCullFace(attributes.CullMode))
	}

	static void GLErrorCallback(int error, const char* description) { PK_CORE_ERROR("GLFW Error (%i) : %s", error, description); }


	void GraphicsAPI::Initialize()
	{
		if (!glfwInit())
		{
			PK_CORE_ERROR("Failed To Initialize GLFW");
		}
	
		glfwSetErrorCallback(GLErrorCallback);
		PK_CORE_LOG_HEADER("GLFW Initialized");
	}
	
	void GraphicsAPI::Terminate()
	{
		glfwTerminate();
		PK_CORE_LOG_HEADER("GLFW Terminated");
	}
	

	void GraphicsAPI::OpenContext(GraphicsContext* context) { m_currentContext = context; }

	void GraphicsAPI::CloseContext() { m_currentContext = nullptr; }


	void GraphicsAPI::StartWindow()
	{
		SetRenderTarget(nullptr);
		Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFrontFace(GL_CW);
	}
	
	void GraphicsAPI::EndWindow() { glfwSwapBuffers(CURRENT_WINDOW); }
	
	
    uint2 GraphicsAPI::GetWindowResolution(GLFWwindow* window)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return int2((uint)width, (uint)height);
	}
	
	uint2 GraphicsAPI::GetActiveWindowResolution() { return GetWindowResolution(CURRENT_WINDOW); }

	float4x4 GraphicsAPI::GetActiveViewProjectionMatrix() { return *GLOBAL_PROPERTIES.GetPropertyPtr<float4x4>(HashCache::Get()->pk_MATRIX_VP); }

	const RenderTexture* GraphicsAPI::GetActiveRenderTarget() { return ACTIVE_RENDERTARGET; }
	
	const RenderTexture* GraphicsAPI::GetBackBuffer() { return nullptr; }

    int GraphicsAPI::GetActiveShaderProgramId()
    {
		GLint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		return currentProgram;
    }
	
	void GraphicsAPI::ResetResourceBindings() { RESOURCE_BINDINGS.ResetBindStates(); }

    void GraphicsAPI::ClearGlobalProperties() { GLOBAL_PROPERTIES.Clear(); }
	void GraphicsAPI::SetGlobalFloat(uint32_t hashId, const float* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat(hashId, values, count); }
	void GraphicsAPI::SetGlobalFloat2(uint32_t hashId, const float2* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat2(hashId, values, count); }
	void GraphicsAPI::SetGlobalFloat3(uint32_t hashId, const float3* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat3(hashId, values, count); }
	void GraphicsAPI::SetGlobalFloat4(uint32_t hashId, const float4* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat4(hashId, values, count); }
	void GraphicsAPI::SetGlobalFloat2x2(uint32_t hashId, const float2x2* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat2x2(hashId, values, count); }
	void GraphicsAPI::SetGlobalFloat3x3(uint32_t hashId, const float3x3* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat3x3(hashId, values, count); }
	void GraphicsAPI::SetGlobalFloat4x4(uint32_t hashId, const float4x4* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat4x4(hashId, values, count); }
	void GraphicsAPI::SetGlobalInt(uint32_t hashId, const int* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt(hashId, values, count); }
	void GraphicsAPI::SetGlobalInt2(uint32_t hashId, const int2* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt2(hashId, values, count); }
	void GraphicsAPI::SetGlobalInt3(uint32_t hashId, const int3* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt3(hashId, values, count); }
	void GraphicsAPI::SetGlobalInt4(uint32_t hashId, const int4* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt4(hashId, values, count); }
	void GraphicsAPI::SetGlobalUInt(uint32_t hashId, const uint* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt(hashId, values, count); }
	void GraphicsAPI::SetGlobalUInt2(uint32_t hashId, const uint2* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt2(hashId, values, count); }
	void GraphicsAPI::SetGlobalUInt3(uint32_t hashId, const uint3* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt3(hashId, values, count); }
	void GraphicsAPI::SetGlobalUInt4(uint32_t hashId, const uint4* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt4(hashId, values, count); }
	void GraphicsAPI::SetGlobalTexture(uint32_t hashId, const GraphicsID* textureIds, uint32_t count) { GLOBAL_PROPERTIES.SetTexture(hashId, textureIds, count); }
	void GraphicsAPI::SetGlobalConstantBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count) { GLOBAL_PROPERTIES.SetConstantBuffer(hashId, bufferIds, count); }
	void GraphicsAPI::SetGlobalComputeBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count) { GLOBAL_PROPERTIES.SetComputeBuffer(hashId, bufferIds, count); }
	
	void GraphicsAPI::SetGlobalFloat(uint32_t hashId, float value) { GLOBAL_PROPERTIES.SetFloat(hashId, value); }
	void GraphicsAPI::SetGlobalFloat2(uint32_t hashId, const float2& value) { GLOBAL_PROPERTIES.SetFloat2(hashId, value); }
	void GraphicsAPI::SetGlobalFloat3(uint32_t hashId, const float3& value) { GLOBAL_PROPERTIES.SetFloat3(hashId, value); }
	void GraphicsAPI::SetGlobalFloat4(uint32_t hashId, const float4& value) { GLOBAL_PROPERTIES.SetFloat4(hashId, value); }
	void GraphicsAPI::SetGlobalFloat2x2(uint32_t hashId, const float2x2& value) { GLOBAL_PROPERTIES.SetFloat2x2(hashId, value); }
	void GraphicsAPI::SetGlobalFloat3x3(uint32_t hashId, const float3x3& value) { GLOBAL_PROPERTIES.SetFloat3x3(hashId, value); }
	void GraphicsAPI::SetGlobalFloat4x4(uint32_t hashId, const float4x4& value) { GLOBAL_PROPERTIES.SetFloat4x4(hashId, value); }
	void GraphicsAPI::SetGlobalInt(uint32_t hashId, int value) { GLOBAL_PROPERTIES.SetInt(hashId, value); }
	void GraphicsAPI::SetGlobalInt2(uint32_t hashId, const int2& value) { GLOBAL_PROPERTIES.SetInt2(hashId, value); }
	void GraphicsAPI::SetGlobalInt3(uint32_t hashId, const int3& value) { GLOBAL_PROPERTIES.SetInt3(hashId, value); }
	void GraphicsAPI::SetGlobalInt4(uint32_t hashId, const int4& value) { GLOBAL_PROPERTIES.SetInt4(hashId, value); }
	void GraphicsAPI::SetGlobalUInt(uint32_t hashId, uint value) { GLOBAL_PROPERTIES.SetUInt(hashId, value); }
	void GraphicsAPI::SetGlobalUInt2(uint32_t hashId, const uint2& value) { GLOBAL_PROPERTIES.SetUInt2(hashId, value); }
	void GraphicsAPI::SetGlobalUInt3(uint32_t hashId, const uint3& value) { GLOBAL_PROPERTIES.SetUInt3(hashId, value); }
	void GraphicsAPI::SetGlobalUInt4(uint32_t hashId, const uint4& value) { GLOBAL_PROPERTIES.SetUInt4(hashId, value); }
	void GraphicsAPI::SetGlobalTexture(uint32_t hashId, GraphicsID textureId) { GLOBAL_PROPERTIES.SetTexture(hashId, textureId); }
	void GraphicsAPI::SetGlobalConstantBuffer(uint32_t hashId, GraphicsID bufferId) { GLOBAL_PROPERTIES.SetConstantBuffer(hashId, bufferId); }
	void GraphicsAPI::SetGlobalComputeBuffer(uint32_t hashId, GraphicsID bufferId) { GLOBAL_PROPERTIES.SetComputeBuffer(hashId, bufferId); }
	void GraphicsAPI::SetGlobalKeyword(uint32_t hashId, bool value) { GLOBAL_PROPERTIES.SetKeyword(hashId, value); }

	void GraphicsAPI::Clear(const float4& color, float depth, GLuint clearFlags)
	{
		auto& attributes = CURRENT_ATTRIBUTES;
		attributes.ZWriteEnabled = true;
		attributes.ColorMask = 255;
		glDepthMask(GL_TRUE);
		glColorMask(true, true, true, true);
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(depth);
		glClear(clearFlags);
	}
	
	void GraphicsAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		auto context = GetCurrentContext();

		if (context->ViewPortX == x && context->ViewPortY == y && context->ViewPortW == width && context->ViewPortH == height)
		{
			return;
		}

		context->ViewPortX = x;
		context->ViewPortY = y;
		context->ViewPortW = width;
		context->ViewPortH = height;

		glViewport(x, y, width, height);
		SetGlobalFloat4(HashCache::Get()->pk_ScreenParams, { (float)width, (float)height, 1.0f + 1.0f / (float)width, 1.0f + 1.0f / (float)height });
	}
	
	void GraphicsAPI::SetViewProjectionMatrices(const float4x4& view, const float4x4& projection)
	{
		auto* hashCache = HashCache::Get();

		auto cameraMatrix = glm::inverse(view);

		auto f = -projection[3][2] / (projection[2][2] - 1.0f);
		auto n = -projection[3][2] / (projection[2][2] + 1.0f);
		auto vp = projection * view;

		SetGlobalFloat4(hashCache->pk_ProjectionParams, { 1.0f, n, f, 1.0f / f });
		SetGlobalFloat4(hashCache->pk_WorldSpaceCameraPos, cameraMatrix[3]);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_V, view);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_V, cameraMatrix);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_P, projection);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_P, glm::inverse(projection));
		SetGlobalFloat4x4(hashCache->pk_MATRIX_VP, vp);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_VP, glm::inverse(vp));
	}
	
	void GraphicsAPI::SetModelMatrix(const float4x4& matrix)
	{
		auto* hashCache = HashCache::Get();
		SetGlobalFloat4x4(hashCache->pk_MATRIX_M, matrix);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_M, glm::inverse(matrix));
	}

	void GraphicsAPI::SetModelMatrix(const float4x4& matrix, const float4x4& invMatrix)
	{
		auto* hashCache = HashCache::Get();
		SetGlobalFloat4x4(hashCache->pk_MATRIX_M, matrix);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_M, invMatrix);
	}
	
	void GraphicsAPI::SetRenderTarget(const RenderTexture* renderTexture)
	{
		auto target = &ACTIVE_RENDERTARGET;

		if (*target == renderTexture)
		{
			return;
		}

		if (renderTexture != nullptr)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, renderTexture->GetGraphicsID());
			*target = renderTexture;
			SetViewPort(0, 0, renderTexture->GetWidth(), renderTexture->GetHeight());
			return;
		}
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*target = nullptr;
		auto resolution = GetActiveWindowResolution();
		SetViewPort(0, 0, resolution.x, resolution.y);
	}
	
	void GraphicsAPI::SetRenderBuffer(const GraphicsID renderTarget, const RenderBuffer* renderBuffer, GLenum attachment)
	{
		glNamedFramebufferTexture(renderTarget, attachment, renderBuffer->GetGraphicsID(), 0);
	}
	
	void GraphicsAPI::SetPass(Shader* shader, uint32_t pass)
	{
		auto* context = GetCurrentContext();

		if (shader != nullptr)
		{
			auto& variant = shader->GetActiveVariant();

			if (context->ActiveShader == variant)
			{
				return;
			}

			context->ActiveShader = variant;
			glUseProgram(variant->GetGraphicsID());
			SetFixedStateAttributes(shader->GetFixedStateAttributes());
			return;
		}

		context->ActiveShader = nullptr;
		glUseProgram(0);
	}
	
	void GraphicsAPI::SetVertexBuffer(const VertexBuffer* buffer) { glBindBuffer(GL_ARRAY_BUFFER, buffer == nullptr ? 0 : buffer->GetGraphicsID()); }

	void GraphicsAPI::SetIndexBuffer(const IndexBuffer* buffer) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer == nullptr ? 0 : buffer->GetGraphicsID()); }


    void GraphicsAPI::BindTextures(ushort location, const GraphicsID* graphicsIds, ushort count) { RESOURCE_BINDINGS.BindTextures(location, graphicsIds, count); }

	void GraphicsAPI::BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsIds, ushort count) { RESOURCE_BINDINGS.BindBuffers(type, location, graphicsIds, count); }
	
	
	void GraphicsAPI::Blit(Shader* shader) { DrawMesh(BLIT_QUAD, 0, shader); }
	
	void GraphicsAPI::Blit(Shader* shader, const ShaderPropertyBlock& propertyBlock) { DrawMesh(BLIT_QUAD, 0, shader, propertyBlock); }
	
	void GraphicsAPI::Blit(const RenderTexture* destination, Shader* shader)
	{
		SetRenderTarget(destination);
		Blit(shader);
	}
	
	void GraphicsAPI::Blit(const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		SetRenderTarget(destination);
		Blit(shader, propertyBlock);
	}
	
	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, BLIT_SHADER);
	}
	
	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, Shader* shader)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, shader);
	}
	
	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, shader, propertyBlock);
	}
	

	void GraphicsAPI::Blit(const Material* material) { DrawMesh(BLIT_QUAD, 0, material); }

	void GraphicsAPI::Blit(const Material* material, const ShaderPropertyBlock& propertyBlock) { DrawMesh(BLIT_QUAD, 0, material, propertyBlock); }

	void GraphicsAPI::Blit(const RenderTexture* destination, const Material* material)
	{
		SetRenderTarget(destination);
		Blit(material);
	}

	void GraphicsAPI::Blit(const RenderTexture* destination, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		SetRenderTarget(destination);
		Blit(material, propertyBlock);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, const Material* material)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, material);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, material, propertyBlock);
	}

	void GraphicsAPI::CopyRenderTexture(const RenderTexture* source, const RenderTexture* destination, GLbitfield mask, GLenum filter)
	{
		uint2 destresolution = uint2(0,0);
		uint32_t destid = 0;

		if (destination != nullptr)
		{
			destresolution = destination->GetResolution2D();
			destid = destination->GetGraphicsID();
		}
		else
		{
			destresolution = GetActiveWindowResolution();
		}

		glBlitNamedFramebuffer(source->GetGraphicsID(), destid, 0, 0, source->GetWidth(), source->GetHeight(), 0, 0, destresolution.x, destresolution.y, mask, filter);
	}


	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh)
	{
		RESOURCE_BINDINGS.BindMesh(mesh->GetGraphicsID());
		auto indexRange = mesh->GetSubmeshIndexRange(submesh);
		glDrawElements(GL_TRIANGLES, indexRange.count, GL_UNSIGNED_INT, (GLvoid*)(size_t)(indexRange.offset * sizeof(GLuint)));
	}
	
	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}
	
	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix, const float4x4& invMatrix)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		SetModelMatrix(matrix, invMatrix);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}
	
	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}
	
	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}


	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader.get());
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader.get());
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix, const float4x4& invMatrix)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader.get());
		SetModelMatrix(matrix, invMatrix);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader.get());
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader.get());
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}


    void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count)
    {
		RESOURCE_BINDINGS.BindMesh(mesh->GetGraphicsID());
		auto indexRange = mesh->GetSubmeshIndexRange(submesh);
		glDrawElementsInstancedBaseInstance(GL_TRIANGLES, indexRange.count, GL_UNSIGNED_INT, (GLvoid*)(size_t)(indexRange.offset * sizeof(GLuint)), count, offset);
    }

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMeshInstanced(mesh, submesh, offset, count);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMeshInstanced(mesh, submesh, offset, count);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader.get());
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMeshInstanced(mesh, submesh, offset, count);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader.get());
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMeshInstanced(mesh, submesh, offset, count);
	}


	void GraphicsAPI::DrawProcedural(Shader* shader, GLenum topology, size_t offset, size_t count)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		glDrawArrays(topology, (GLint)offset, (GLsizei)count);
	}

	void GraphicsAPI::DispatchCompute(Shader* shader, uint3 threadGroupSize)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);

		glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void GraphicsAPI::DispatchCompute(const Material* material, uint3 threadGroupSize)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader.get());
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);

		glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void GraphicsAPI::DispatchCompute(Shader* shader, uint3 threadGroupSize, const ShaderPropertyBlock& propertyBlock)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);

		glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

    void GraphicsAPI::DispatchComputeIndirect(Shader* shader, const GraphicsID& argumentsBuffer, uint offset, const ShaderPropertyBlock& propertyBlock)
    {
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);

		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, argumentsBuffer);
		glDispatchComputeIndirect(offset);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    }

	#undef CURRENT_WINDOW 
	#undef CURRENT_ATTRIBUTES 
	#undef GLOBAL_KEYWORDS 
	#undef GLOBAL_PROPERTIES 
	#undef RESOURCE_BINDINGS 
	#undef ACTIVE_RENDERTARGET 
	#undef BLIT_QUAD 
	#undef BLIT_SHADER 
	#undef DELTA_CHECK_SET
}