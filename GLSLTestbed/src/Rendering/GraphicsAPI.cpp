#include "PrecompiledHeader.h"
#include "Utilities/HashCache.h"
#include "Rendering/GraphicsAPI.h"
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
	#define BLIT_SHADER GetCurrentContext()->BlitShader
	#define DELTA_CHECK_SET(backedvalue, newvalue, func)  \
	{													  \
		if (backedvalue != newvalue)				      \
		{												  \
			backedvalue = newvalue;						  \
			func;										  \
		}												  \
	}													  \

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
		auto& current = CURRENT_ATTRIBUTES;
		DELTA_CHECK_SET(current.ZTestEnabled, attributes.ZTestEnabled, glToggle(GL_DEPTH_TEST, attributes.ZTestEnabled))
		DELTA_CHECK_SET(current.BlendEnabled, attributes.BlendEnabled, glToggle(GL_BLEND, attributes.BlendEnabled))
		DELTA_CHECK_SET(current.CullEnabled, attributes.CullEnabled, glToggle(GL_CULL_FACE, attributes.CullEnabled))
		DELTA_CHECK_SET(current.ZWriteEnabled, attributes.ZWriteEnabled, glDepthMask(attributes.ZWriteEnabled))
		DELTA_CHECK_SET(current.ColorMask, attributes.ColorMask, glColorMask(
			(attributes.ColorMask & (1 << 0)) != 0, 
			(attributes.ColorMask & (1 << 1)) != 0, 
			(attributes.ColorMask & (1 << 2)) != 0, 
			(attributes.ColorMask & (1 << 3)) != 0))

		if (attributes.ZTestEnabled)
		{
			DELTA_CHECK_SET(current.ZTest, attributes.ZTest, glDepthFunc(attributes.ZTest))
		}

		if (attributes.BlendEnabled)
		{
			DELTA_CHECK_SET(current.Blend, attributes.Blend, glBlendFunc(attributes.Blend.Source, attributes.Blend.Destination))
		}

		if (attributes.CullEnabled)
		{
			DELTA_CHECK_SET(current.CullMode, attributes.CullMode, glCullFace(attributes.CullMode))
		}
	}
	

	void GraphicsAPI::Initialize()
	{
		if (!glfwInit())
		{
			PK_CORE_ERROR("Failed To Initialize GLFW");
		}
	
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

	int GraphicsAPI::GetMemoryUsageKB()
	{
		#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
		#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

		GLint total_mem_kb = 0;
		GLint cur_avail_mem_kb = 0;

		glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
		glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);

		return total_mem_kb - cur_avail_mem_kb;
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
	void GraphicsAPI::SetGlobalImage(uint32_t hashId, const ImageBindDescriptor* imageBindings, uint32_t count) { GLOBAL_PROPERTIES.SetImage(hashId, imageBindings, count); }
	void GraphicsAPI::SetGlobalConstantBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count) { GLOBAL_PROPERTIES.SetConstantBuffer(hashId, bufferIds, count); }
	void GraphicsAPI::SetGlobalComputeBuffer(uint32_t hashId, const GraphicsID* bufferIds, uint32_t count) { GLOBAL_PROPERTIES.SetComputeBuffer(hashId, bufferIds, count); }
	void GraphicsAPI::SetGlobalResourceHandle(uint32_t hashId, const ulong* handleIds, uint32_t count) { GLOBAL_PROPERTIES.SetResourceHandle(hashId, handleIds, count); }
	
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
	void GraphicsAPI::SetGlobalImage(uint32_t hashId, const ImageBindDescriptor& imageBindings) { GLOBAL_PROPERTIES.SetImage(hashId, imageBindings); }
	void GraphicsAPI::SetGlobalConstantBuffer(uint32_t hashId, GraphicsID bufferId) { GLOBAL_PROPERTIES.SetConstantBuffer(hashId, bufferId); }
	void GraphicsAPI::SetGlobalComputeBuffer(uint32_t hashId, GraphicsID bufferId) { GLOBAL_PROPERTIES.SetComputeBuffer(hashId, bufferId); }
	void GraphicsAPI::SetGlobalResourceHandle(uint32_t hashId, const ulong handleId) { GLOBAL_PROPERTIES.SetResourceHandle(hashId, handleId); }
	void GraphicsAPI::SetGlobalKeyword(uint32_t hashId, bool value) { GLOBAL_PROPERTIES.SetKeyword(hashId, value); }

	void GraphicsAPI::Clear(const float4& color, float depth, GLuint clearFlags)
	{
		auto context = GetCurrentContext();
		auto& attributes = context->FixedStateAttributes;
		DELTA_CHECK_SET(attributes.ZWriteEnabled, true, glDepthMask(true))
		DELTA_CHECK_SET(attributes.ColorMask, 255, glColorMask(true,true,true,true))
		DELTA_CHECK_SET(context->ClearColor, color, glClearColor(color.r, color.g, color.b, color.a))
		DELTA_CHECK_SET(context->ClearDepth, depth, glClearDepth(depth))
		glClear(clearFlags);
	}

	void GraphicsAPI::ResetViewPort()
	{
		auto target = ACTIVE_RENDERTARGET;

		if (target != nullptr)
		{
			SetViewPort(0, 0, target->GetWidth(), target->GetHeight());
			return;
		}

		auto resolution = GetActiveWindowResolution();
		SetViewPort(0, 0, resolution.x, resolution.y);
	}
	
	void GraphicsAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		auto context = GetCurrentContext();

		if (context->ViewPort.x == x && context->ViewPort.y == y && context->ViewPort.z == width && context->ViewPort.w == height)
		{
			return;
		}

		context->ViewPort.x = x;
		context->ViewPort.y = y;
		context->ViewPort.z = width;
		context->ViewPort.w = height;

		glViewport(x, y, width, height);
		SetGlobalFloat4(HashCache::Get()->pk_ScreenParams, { (float)width, (float)height, 1.0f / (float)width, 1.0f / (float)height });
	}

	void GraphicsAPI::SetViewPorts(const uint32_t offset, const float4* viewports, const uint32_t count)
	{
		auto context = GetCurrentContext();
		context->ViewPort.x = (uint)viewports->x;
		context->ViewPort.y = (uint)viewports->y;
		context->ViewPort.z = (uint)viewports->z;
		context->ViewPort.w = (uint)viewports->w;
		glViewportArrayv(offset, (GLsizei)count, reinterpret_cast<const float*>(viewports));
	}
	
	void GraphicsAPI::SetViewProjectionMatrices(const float4x4& view, const float4x4& projection)
	{
		auto* hashCache = HashCache::Get();

		auto cameraMatrix = glm::inverse(view);

		auto n = Functions::GetZNearFromProj(projection);
		auto f = Functions::GetZFarFromProj(projection);
		auto vp = projection * view;

		SetGlobalFloat4(hashCache->pk_ProjectionParams, { n, f, f - n, 1.0f / f });
		SetGlobalFloat4(hashCache->pk_ExpProjectionParams, { 1.0f / glm::log2(f/n), -log2(n) / log2(f/n), f/n, 1.0f/n });
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
	
	void GraphicsAPI::SetRenderTarget(const RenderTexture* renderTexture, bool updateViewport)
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
			
			if (updateViewport)
			{
				SetViewPort(0, 0, renderTexture->GetWidth(), renderTexture->GetHeight());
			}

			return;
		}
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*target = nullptr;

		if (updateViewport)
		{
			auto resolution = GetActiveWindowResolution();
			SetViewPort(0, 0, resolution.x, resolution.y);
		}
	}

	void GraphicsAPI::SetRenderTarget(const RenderTexture* renderTexture, const uint firstViewport, const float4* viewports, const uint viewportCount)
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
			SetViewPorts(firstViewport, viewports, viewportCount);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*target = nullptr;
		auto resolution = GetActiveWindowResolution();
		SetViewPorts(firstViewport, viewports, viewportCount);
	}
	
	void GraphicsAPI::SetRenderBuffer(const GraphicsID renderTarget, const RenderBuffer* renderBuffer, GLenum attachment)
	{
		glNamedFramebufferTexture(renderTarget, attachment, renderBuffer->GetGraphicsID(), 0);
	}
	
	void GraphicsAPI::SetPass(Shader* shader, const FixedStateAttributes& attributes, uint32_t pass)
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
			SetFixedStateAttributes(attributes);
			return;
		}

		context->ActiveShader = nullptr;
		glUseProgram(0);
	}
	
	void GraphicsAPI::SetVertexBuffer(const VertexBuffer* buffer) { glBindBuffer(GL_ARRAY_BUFFER, buffer == nullptr ? 0 : buffer->GetGraphicsID()); }

	void GraphicsAPI::SetIndexBuffer(const IndexBuffer* buffer) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer == nullptr ? 0 : buffer->GetGraphicsID()); }


    void GraphicsAPI::BindTextures(ushort location, const GraphicsID* graphicsIds, ushort count) { RESOURCE_BINDINGS.BindTextures(location, graphicsIds, count); }

	void GraphicsAPI::BindImages(ushort location, const ImageBindDescriptor* imageBindings, ushort count) { RESOURCE_BINDINGS.BindImages(location, imageBindings, count); }

	void GraphicsAPI::BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsIds, ushort count) { RESOURCE_BINDINGS.BindBuffers(type, location, graphicsIds, count); }

	void GraphicsAPI::SetMemoryBarrier(GLenum barrierFlags) 
	{ 
		if (barrierFlags != 0)
		{
			glMemoryBarrier(barrierFlags); 
		}
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

	void GraphicsAPI::ExecuteDrawCall(const DrawCallDescriptor& descriptor)
	{
		if (descriptor.source != nullptr)
		{
			SetGlobalTexture(HashCache::Get()->_MainTex, descriptor.source->GetGraphicsID());
		}

		if (descriptor.matrix != nullptr)
		{
			if (descriptor.invMatrix != nullptr)
			{
				SetModelMatrix(*descriptor.matrix, *descriptor.invMatrix);
			}
			else
			{
				SetModelMatrix(*descriptor.matrix);
			}
		}

		if (descriptor.shader != nullptr)
		{
			descriptor.shader->ResetKeywords();

			if (descriptor.propertyBlock0 != nullptr)
			{
				descriptor.shader->SetKeywords(descriptor.propertyBlock0->GetKeywords());
			}

			descriptor.shader->SetKeywords(GLOBAL_KEYWORDS);

			if (descriptor.propertyBlock1 != nullptr)
			{
				descriptor.shader->SetKeywords(descriptor.propertyBlock1->GetKeywords());
			}

			if (descriptor.attributes != nullptr)
			{
				SetPass(descriptor.shader, *descriptor.attributes);
			}
			else
			{
				SetPass(descriptor.shader, descriptor.shader->GetFixedStateAttributes());
			}

			if (descriptor.propertyBlock0 != nullptr)
			{
				descriptor.shader->SetPropertyBlock(*descriptor.propertyBlock0);
			}

			descriptor.shader->SetPropertyBlock(GLOBAL_PROPERTIES);

			if (descriptor.propertyBlock1 != nullptr)
			{
				descriptor.shader->SetPropertyBlock(*descriptor.propertyBlock1);
			}
		}

		if (descriptor.destination != nullptr)
		{
			SetRenderTarget(descriptor.destination);
		}

		if (descriptor.mesh != nullptr)
		{
			RESOURCE_BINDINGS.BindMesh(descriptor.mesh->GetGraphicsID());
		}

		if (descriptor.argumentsBufferId != 0)
		{
			glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, descriptor.argumentsBufferId);
		}

		switch (descriptor.command)
		{
			case DrawCommand::Mesh:
			{
				auto indexRange = descriptor.mesh->GetSubmeshIndexRange(descriptor.submesh);
				glDrawElements(GL_TRIANGLES, indexRange.count, GL_UNSIGNED_INT, (GLvoid*)(size_t)(indexRange.offset * sizeof(GLuint)));
				break;
			}
			case DrawCommand::MeshInstanced:
			{
				auto indexRange = descriptor.mesh->GetSubmeshIndexRange(descriptor.submesh);
				glDrawElementsInstancedBaseInstance(GL_TRIANGLES, indexRange.count, GL_UNSIGNED_INT, (GLvoid*)(size_t)(indexRange.offset * sizeof(GLuint)), (GLsizei)descriptor.count, (GLuint)descriptor.offset);
				break;
			}
			case DrawCommand::Procedural:
			{
				glDrawArrays(descriptor.topology, (GLint)descriptor.offset, (GLsizei)descriptor.count);
				break;
			}
			case DrawCommand::Compute:
			{
				glDispatchCompute(descriptor.threadGroupSize.x, descriptor.threadGroupSize.y, descriptor.threadGroupSize.z);
				break;
			}
			case DrawCommand::ComputeIndirect:
			{
				glDispatchComputeIndirect(descriptor.offset);
				break;
			}
		}

		SetMemoryBarrier(descriptor.memoryBarrierFlags);
	}
	

	void GraphicsAPI::Blit(Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = BLIT_SHADER;
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const RenderTexture* destination, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void Blit(const RenderTexture* destination, Shader* shader, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void Blit(const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void Blit(const Texture* source, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Material* material)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = BLIT_QUAD;
		descriptor.propertyBlock0 = material;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = BLIT_QUAD;
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const RenderTexture* destination, const Material* material)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.propertyBlock0 = material;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}
	
	void GraphicsAPI::Blit(const RenderTexture* destination, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, const Material* material)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.propertyBlock0 = material;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::Blit(const Texture* source, const RenderTexture* destination, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::BlitInstanced(uint offset, uint count, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::BlitInstanced(uint offset, uint count, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void BlitInstanced(uint offset, uint count, Shader* shader, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::BlitInstanced(uint offset, uint count, const RenderTexture* destination, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::BlitInstanced(uint offset, uint count, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void BlitInstanced(uint offset, uint count, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::BlitInstanced(uint offset, uint count, const Texture* source, const RenderTexture* destination, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::BlitInstanced(uint offset, uint count, const Texture* source, const RenderTexture* destination, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = BLIT_QUAD;
		descriptor.source = source;
		descriptor.destination = destination;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh)
	{
		DrawCallDescriptor descriptor;
		descriptor.mesh = mesh;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.matrix = &matrix;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix, const float4x4& invMatrix)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.matrix = &matrix;
		descriptor.invMatrix = &invMatrix;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, Shader* shader, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.matrix = &matrix;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.matrix = &matrix;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix, const float4x4& invMatrix)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.matrix = &matrix;
		descriptor.invMatrix = &invMatrix;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMesh(const Mesh* mesh, int submesh, const Material* material, const float4x4& matrix, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.matrix = &matrix;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::Mesh;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count)
	{
		DrawCallDescriptor descriptor;
		descriptor.mesh = mesh;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader, const FixedStateAttributes& attributes)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.attributes = &attributes;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, Shader* shader, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.mesh = mesh;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material, const FixedStateAttributes& attributes)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.attributes = &attributes;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawMeshInstanced(const Mesh* mesh, int submesh, uint offset, uint count, const Material* material, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.mesh = mesh;
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.submesh = submesh;
		descriptor.command = DrawCommand::MeshInstanced;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawProcedural(Shader* shader, GLenum topology, size_t offset, size_t count)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.topology = topology;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::Procedural;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawProcedural(Shader* shader, GLenum topology, size_t offset, size_t count, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.topology = topology;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::Procedural;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DrawProcedural(const Material* material, GLenum topology, size_t offset, size_t count, const ShaderPropertyBlock& propertyBlock)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.topology = topology;
		descriptor.offset = offset;
		descriptor.count = count;
		descriptor.command = DrawCommand::Procedural;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchCompute(Shader* shader, uint3 threadGroupSize, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.threadGroupSize = threadGroupSize;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Compute;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchCompute(Shader* shader, uint3 threadGroupSize, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.threadGroupSize = threadGroupSize;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Compute;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchCompute(const Material* material, uint3 threadGroupSize, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.propertyBlock0 = material;
		descriptor.threadGroupSize = threadGroupSize;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Compute;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchCompute(const Material* material, uint3 threadGroupSize, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.threadGroupSize = threadGroupSize;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Compute;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchComputeIndirect(Shader* shader, const GraphicsID& argumentsBuffer, uint offset, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.offset = offset;
		descriptor.argumentsBufferId = argumentsBuffer;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Compute;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchComputeIndirect(Shader* shader, const GraphicsID& argumentsBuffer, uint offset, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = shader;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.argumentsBufferId = argumentsBuffer;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::Compute;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchComputeIndirect(const Material* material, const GraphicsID& argumentsBuffer, uint offset, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.propertyBlock0 = material;
		descriptor.offset = offset;
		descriptor.argumentsBufferId = argumentsBuffer;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::ComputeIndirect;
		ExecuteDrawCall(descriptor);
	}

	void GraphicsAPI::DispatchComputeIndirect(const Material* material, const GraphicsID& argumentsBuffer, uint offset, const ShaderPropertyBlock& propertyBlock, GLenum barrierFlags)
	{
		DrawCallDescriptor descriptor;
		descriptor.shader = material->GetShader();
		descriptor.propertyBlock0 = material;
		descriptor.propertyBlock1 = &propertyBlock;
		descriptor.offset = offset;
		descriptor.argumentsBufferId = argumentsBuffer;
		descriptor.memoryBarrierFlags = barrierFlags;
		descriptor.command = DrawCommand::ComputeIndirect;
		ExecuteDrawCall(descriptor);
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