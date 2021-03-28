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
	#define BLIT_QUAD GetCurrentContext()->BlitQuad
	#define BLIT_SHADER GetCurrentContext()->BlitShader.lock()
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
		DELTA_CHECK_SET(attributes, CullMode, glCullFace(attributes.CullMode))
	}

	static void GLErrorCallback(int error, const char* description) { PK_CORE_ERROR("GLFW Error (%i) : %s", error, description); }


	void Initialize()
	{
		if (!glfwInit())
		{
			PK_CORE_ERROR("Failed To Initialize GLFW");
		}
	
		glfwSetErrorCallback(GLErrorCallback);
		PK_CORE_LOG_HEADER("GLFW Initialized");
	}
	
	void Terminate()
	{
		glfwTerminate();
		PK_CORE_LOG_HEADER("GLFW Terminated");
	}
	

	void OpenContext(GraphicsContext* context) { m_currentContext = context; }

	void CloseContext() { m_currentContext = nullptr; }


	void StartWindow()
	{
		SetRenderTarget(nullptr);
		Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFrontFace(GL_CW);
	}
	
	void EndWindow() { glfwSwapBuffers(CURRENT_WINDOW); }
	
	
    uint2 GetWindowResolution(GLFWwindow* window)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return int2((uint)width, (uint)height);
	}
	
	uint2 GetActiveWindowResolution() { return GetWindowResolution(CURRENT_WINDOW); }

	float4x4 GetActiveViewProjectionMatrix() { return *GLOBAL_PROPERTIES.GetPropertyPtr<float4x4>(HashCache::Get()->pk_MATRIX_VP); }

	Ref<RenderTexture>& GetActiveRenderTarget() { return ACTIVE_RENDERTARGET; }
	
	Ref<RenderTexture> GetBackBuffer() { return (Ref<RenderTexture>)nullptr; }

    int GetActiveShaderProgramId()
    {
		GLint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		return currentProgram;
    }
	
	void ResetResourceBindings() { RESOURCE_BINDINGS.ResetBindStates(); }

    void ClearGlobalProperties() { GLOBAL_PROPERTIES.Clear(); }
	void SetGlobalFloat(uint32_t hashId, const float* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat(hashId, values, count); }
	void SetGlobalFloat2(uint32_t hashId, const float2* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat2(hashId, values, count); }
	void SetGlobalFloat3(uint32_t hashId, const float3* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat3(hashId, values, count); }
	void SetGlobalFloat4(uint32_t hashId, const float4* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat4(hashId, values, count); }
	void SetGlobalFloat2x2(uint32_t hashId, const float2x2* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat2x2(hashId, values, count); }
	void SetGlobalFloat3x3(uint32_t hashId, const float3x3* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat3x3(hashId, values, count); }
	void SetGlobalFloat4x4(uint32_t hashId, const float4x4* values, uint32_t count) { GLOBAL_PROPERTIES.SetFloat4x4(hashId, values, count); }
	void SetGlobalInt(uint32_t hashId, const int* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt(hashId, values, count); }
	void SetGlobalInt2(uint32_t hashId, const int2* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt2(hashId, values, count); }
	void SetGlobalInt3(uint32_t hashId, const int3* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt3(hashId, values, count); }
	void SetGlobalInt4(uint32_t hashId, const int4* values, uint32_t count) { GLOBAL_PROPERTIES.SetInt4(hashId, values, count); }
	void SetGlobalUInt(uint32_t hashId, const uint* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt(hashId, values, count); }
	void SetGlobalUInt2(uint32_t hashId, const uint2* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt2(hashId, values, count); }
	void SetGlobalUInt3(uint32_t hashId, const uint3* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt3(hashId, values, count); }
	void SetGlobalUInt4(uint32_t hashId, const uint4* values, uint32_t count) { GLOBAL_PROPERTIES.SetUInt4(hashId, values, count); }
	void SetGlobalTexture(uint32_t hashId, const GraphicsID* textureIds, uint32_t count) { GLOBAL_PROPERTIES.SetTexture(hashId, textureIds, count); }
	void SetGlobalConstantBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count) { GLOBAL_PROPERTIES.SetConstantBuffer(hashId, bufferIds, count); }
	void SetGlobalComputeBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count) { GLOBAL_PROPERTIES.SetComputeBuffer(hashId, bufferIds, count); }
	
	void SetGlobalFloat(uint32_t hashId, float value) { GLOBAL_PROPERTIES.SetFloat(hashId, value); }
	void SetGlobalFloat2(uint32_t hashId, const float2& value) { GLOBAL_PROPERTIES.SetFloat2(hashId, value); }
	void SetGlobalFloat3(uint32_t hashId, const float3& value) { GLOBAL_PROPERTIES.SetFloat3(hashId, value); }
	void SetGlobalFloat4(uint32_t hashId, const float4& value) { GLOBAL_PROPERTIES.SetFloat4(hashId, value); }
	void SetGlobalFloat2x2(uint32_t hashId, const float2x2& value) { GLOBAL_PROPERTIES.SetFloat2x2(hashId, value); }
	void SetGlobalFloat3x3(uint32_t hashId, const float3x3& value) { GLOBAL_PROPERTIES.SetFloat3x3(hashId, value); }
	void SetGlobalFloat4x4(uint32_t hashId, const float4x4& value) { GLOBAL_PROPERTIES.SetFloat4x4(hashId, value); }
	void SetGlobalInt(uint32_t hashId, int value) { GLOBAL_PROPERTIES.SetInt(hashId, value); }
	void SetGlobalInt2(uint32_t hashId, const int2& value) { GLOBAL_PROPERTIES.SetInt2(hashId, value); }
	void SetGlobalInt3(uint32_t hashId, const int3& value) { GLOBAL_PROPERTIES.SetInt3(hashId, value); }
	void SetGlobalInt4(uint32_t hashId, const int4& value) { GLOBAL_PROPERTIES.SetInt4(hashId, value); }
	void SetGlobalUInt(uint32_t hashId, uint value) { GLOBAL_PROPERTIES.SetUInt(hashId, value); }
	void SetGlobalUInt2(uint32_t hashId, const uint2& value) { GLOBAL_PROPERTIES.SetUInt2(hashId, value); }
	void SetGlobalUInt3(uint32_t hashId, const uint3& value) { GLOBAL_PROPERTIES.SetUInt3(hashId, value); }
	void SetGlobalUInt4(uint32_t hashId, const uint4& value) { GLOBAL_PROPERTIES.SetUInt4(hashId, value); }
	void SetGlobalTexture(uint32_t hashId, GraphicsID textureId) { GLOBAL_PROPERTIES.SetTexture(hashId, textureId); }
	void SetGlobalConstantBuffer(uint32_t hashId, GraphicsID bufferId) { GLOBAL_PROPERTIES.SetConstantBuffer(hashId, bufferId); }
	void SetGlobalComputeBuffer(uint32_t hashId, GraphicsID bufferId) { GLOBAL_PROPERTIES.SetComputeBuffer(hashId, bufferId); }
	void SetGlobalKeyword(uint32_t hashId, bool value) { GLOBAL_PROPERTIES.SetKeyword(hashId, value); }

	void Clear(const float4& color, float depth, GLuint clearFlags)
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
	
	void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
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
	
	void SetViewProjectionMatrices(const float4x4& view, const float4x4& projection)
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
	
	void SetModelMatrix(const float4x4& matrix)
	{
		auto* hashCache = HashCache::Get();
		SetGlobalFloat4x4(hashCache->pk_MATRIX_M, matrix);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_M, glm::inverse(matrix));
	}

	void SetModelMatrix(const float4x4& matrix, const float4x4& invMatrix)
	{
		auto* hashCache = HashCache::Get();
		SetGlobalFloat4x4(hashCache->pk_MATRIX_M, matrix);
		SetGlobalFloat4x4(hashCache->pk_MATRIX_I_M, invMatrix);
	}
	
	void SetRenderTarget(const Ref<RenderTexture>& renderTexture)
	{
		auto& target = ACTIVE_RENDERTARGET;

		if (target == renderTexture)
		{
			return;
		}

		if (renderTexture != nullptr)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, renderTexture->GetGraphicsID());
			target = renderTexture;
			SetViewPort(0, 0, renderTexture->GetWidth(), renderTexture->GetHeight());
			return;
		}
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		target = nullptr;
		auto resolution = GetActiveWindowResolution();
		SetViewPort(0, 0, resolution.x, resolution.y);
	}
	
	void SetRenderBuffer(const GraphicsID renderTarget, const Ref<RenderBuffer>& renderBuffer, GLenum attachment)
	{
		glNamedFramebufferTexture(renderTarget, attachment, renderBuffer->GetGraphicsID(), 0);
	}
	
	void SetPass(const Ref<Shader>& shader, uint32_t pass)
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
			glUseProgram(variant->GetGraphicsID() );
			SetFixedStateAttributes(shader->GetFixedStateAttributes());
			return;
		}

		context->ActiveShader = nullptr;
		glUseProgram(0);
	}
	
	void SetVertexBuffer(const Ref<VertexBuffer>& buffer) { glBindBuffer(GL_ARRAY_BUFFER, buffer == nullptr ? 0 : buffer->GetGraphicsID()); }

	void SetIndexBuffer(const Ref<IndexBuffer>& buffer) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer == nullptr ? 0 : buffer->GetGraphicsID()); }


    void BindTextures(ushort location, const GraphicsID* graphicsIds, ushort count) { RESOURCE_BINDINGS.BindTextures(location, graphicsIds, count); }

	void BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsIds, ushort count) { RESOURCE_BINDINGS.BindBuffers(type, location, graphicsIds, count); }
	
	
	void Blit(const Ref<Shader>& shader)
	{
		DrawMesh(BLIT_QUAD, 0, shader);
	}
	
	void Blit(const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawMesh(BLIT_QUAD, 0, shader, propertyBlock);
	}
	
	void Blit(const Ref<RenderTexture>& destination, const Ref<Shader>& shader)
	{
		SetRenderTarget(destination);
		Blit(shader);
	}
	
	void Blit(const Ref<RenderTexture>& destination, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock)
	{
		SetRenderTarget(destination);
		Blit(shader, propertyBlock);
	}
	
	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, BLIT_SHADER);
	}
	
	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination, const Ref<Shader>& shader)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, shader);
	}
	
	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, shader, propertyBlock);
	}
	

	void Blit(const Ref<Material>& material)
	{
		DrawMesh(BLIT_QUAD, 0, material);
	}

	void Blit(const Ref<Material>& material, const ShaderPropertyBlock& propertyBlock)
	{
		DrawMesh(BLIT_QUAD, 0, material, propertyBlock);
	}

	void Blit(const Ref<RenderTexture>& destination, const Ref<Material>& material)
	{
		SetRenderTarget(destination);
		Blit(material);
	}

	void Blit(const Ref<RenderTexture>& destination, const Ref<Material>& material, const ShaderPropertyBlock& propertyBlock)
	{
		SetRenderTarget(destination);
		Blit(material, propertyBlock);
	}

	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination, const Ref<Material>& material)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, material);
	}

	void Blit(const Ref<Texture>& source, const Ref<RenderTexture>& destination, const Ref<Material>& material, const ShaderPropertyBlock& propertyBlock)
	{
		SetGlobalTexture(HashCache::Get()->_MainTex, source->GetGraphicsID());
		Blit(destination, material, propertyBlock);
	}

	void CopyRenderTexture(const Ref<RenderTexture>& source, const Ref<RenderTexture>& destination, GLbitfield mask, GLenum filter)
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


	void DrawMesh(const Ref<Mesh>& mesh, int submesh)
	{
		RESOURCE_BINDINGS.BindMesh(mesh->GetGraphicsID());
		auto indexRange = mesh->GetSubmeshIndexRange(submesh);
		glDrawElements(GL_TRIANGLES, indexRange.count, GL_UNSIGNED_INT, (GLvoid*)(size_t)(indexRange.offset * sizeof(GLuint)));
	}
	
	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Shader>& shader)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}
	
	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Shader>& shader, const float4x4& matrix)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Shader>& shader, const float4x4& matrix, const float4x4& invMatrix)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		SetModelMatrix(matrix, invMatrix);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}
	
	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}
	
	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Shader>& shader, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock)
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


	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Material>& material)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Material>& material, const float4x4& matrix)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Material>& material, const float4x4& matrix, const float4x4& invMatrix)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		SetModelMatrix(matrix, invMatrix);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMesh(mesh, submesh);
	}

	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Material>& material, const ShaderPropertyBlock& propertyBlock)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}

	void DrawMesh(const Ref<Mesh>& mesh, int submesh, const Ref<Material>& material, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		SetModelMatrix(matrix);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMesh(mesh, submesh);
	}


    void DrawMeshInstanced(const Ref<Mesh>& mesh, int submesh, uint count)
    {
		RESOURCE_BINDINGS.BindMesh(mesh->GetGraphicsID());
		auto indexRange = mesh->GetSubmeshIndexRange(submesh);
		glDrawElementsInstanced(GL_TRIANGLES, indexRange.count, GL_UNSIGNED_INT, (GLvoid*)(size_t)(indexRange.offset * sizeof(GLuint)), count);
    }

	void DrawMeshInstanced(const Ref<Mesh>& mesh, int submesh, uint count, const Ref<Shader>& shader)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMeshInstanced(mesh, submesh, count);
	}

	void DrawMeshInstanced(const Ref<Mesh>& mesh, int submesh, uint count, const Ref<Shader>& shader, const ShaderPropertyBlock& propertyBlock)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMeshInstanced(mesh, submesh, count);
	}


	void DrawMeshInstanced(const Ref<Mesh>& mesh, int submesh, uint count, const Ref<Material>& material)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		DrawMeshInstanced(mesh, submesh, count);
	}

	void DrawMeshInstanced(const Ref<Mesh>& mesh, int submesh, uint count, const Ref<Material>& material, const ShaderPropertyBlock& propertyBlock)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		shader->SetKeywords(propertyBlock.GetKeywords());
		SetPass(shader);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		shader->SetPropertyBlock(propertyBlock);
		DrawMeshInstanced(mesh, submesh, count);
	}

	void DrawProcedural(const Ref<Shader>& shader, GLenum topology, size_t offset, size_t count)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);
		glDrawArrays(topology, (GLint)offset, (GLsizei)count);
	}

	void DispatchCompute(const Ref<Shader>& shader, uint3 threadGroupSize)
	{
		shader->ResetKeywords();
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);

		glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void DispatchCompute(const Ref<Material>& material, uint3 threadGroupSize)
	{
		auto shader = material->GetShader().lock();
		shader->ResetKeywords();
		shader->SetKeywords(material->GetKeywords());
		shader->SetKeywords(GLOBAL_KEYWORDS);
		SetPass(shader);
		shader->SetPropertyBlock(*material);
		shader->SetPropertyBlock(GLOBAL_PROPERTIES);

		glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void DispatchCompute(const Ref<Shader>& shader, uint3 threadGroupSize, const ShaderPropertyBlock& propertyBlock)
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

    void DispatchComputeIndirect(const Ref<Shader>& shader, const GraphicsID& argumentsBuffer, uint offset, const ShaderPropertyBlock& propertyBlock)
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