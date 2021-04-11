#include "PrecompiledHeader.h"
#include "FilterBloom.h"
#include "Utilities/HashCache.h"
#include "Rendering/Graphics.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    struct PassParams
    {
        ulong source;
        float2 offset;
        uint2 readwrite;
    };

    FilterBloom::FilterBloom(Weak<Shader> shader, Weak<TextureXD> lensDirt, float exposure, float intensity, float lensDirtIntensity) : FilterBase(shader)
    {
        m_exposure = exposure;
        m_intensity = intensity;
        m_lensDirtIntensity = lensDirtIntensity;
        m_lensDirtTexture = lensDirt;
    
        m_passKeywords[0] = StringHashID::StringToID("BLOOM_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("BLOOM_PASS1");
        m_passKeywords[2] = StringHashID::StringToID("BLOOM_PASS2");
    }
    
    static void UpdateBufferData(const RenderTexture* source, 
        Ref<RenderTexture>* renderTextures, 
        Ref<ComputeBuffer>& passBuffer, 
        Weak<TextureXD>& lensdirt, 
        float bloomIntensity, 
        float dirtIntensity, 
        float exposure,
        float saturation)
    {
        auto descriptor = source->GetCompoundDescriptor();
        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.resolution.z = 2;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;

        auto hasdelta = false;

        for (int i = 0; i < 6; ++i)
        {
            descriptor.resolution.x /= 2;
            descriptor.resolution.y /= 2;
    
            if (renderTextures[i] == nullptr)
            {
                renderTextures[i] = CreateRef<RenderTexture>(descriptor);
                hasdelta = true;
            }
            else
            {
                hasdelta |= renderTextures[i]->ValidateResolution(descriptor.resolution);
            }
        }

        if (passBuffer == nullptr)
        {
            passBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::HANDLE, "SOURCE"}, { CG_TYPE::FLOAT2, "OFFSET" }, { CG_TYPE::UINT2, "READWRITE" } }), 32, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        }

        if (!hasdelta)
        {
            return;
        }

        GLuint64 handles[8] =
        {
            renderTextures[0]->GetColorBufferPtr(0)->GetBindlessHandleResident(),
            renderTextures[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(),
            renderTextures[2]->GetColorBufferPtr(0)->GetBindlessHandleResident(),
            renderTextures[3]->GetColorBufferPtr(0)->GetBindlessHandleResident(),
            renderTextures[4]->GetColorBufferPtr(0)->GetBindlessHandleResident(),
            renderTextures[5]->GetColorBufferPtr(0)->GetBindlessHandleResident(),
            source->GetColorBufferPtr(0)->GetBindlessHandleResident(),//Kinda volatile not to do this every frame but whatever.
            lensdirt.lock()->GetBindlessHandleResident(),
        };

        const float blurSize = 4.0f;
        auto bufferview = passBuffer->BeginMapBuffer<PassParams>();
        auto passIdx = 0;

        for (int i = 0; i < 6; ++i)
        {
            bufferview[passIdx++] = { i == 0 ? handles[6] : handles[i - 1], CG_FLOAT2_ZERO, { 0, 0 } };
            auto spread = i == 2 ? 0.75f : (i > 1 ? 1.0f : 0.5f);

            for (int j = 0; j < 2; ++j)
            {
                bufferview[passIdx++] = { handles[i], { (blurSize * 0.5f + j) * spread, 0 }, { 0, 1 }};
                bufferview[passIdx++] = { handles[i], { 0, (blurSize * 0.5f + j) * spread }, { 1, 0 }};
            }
        }

        bufferview[passIdx++] = { handles[6], { glm::exp(bloomIntensity) - 1.0f, glm::exp(dirtIntensity) - 1.0f }, { 0, 0} };
        bufferview[passIdx++] = { handles[7], { exposure, saturation }, { 0, 0 }};

        passBuffer->EndMapBuffer();
    }
    
    void FilterBloom::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        UpdateBufferData(source, m_blurTextures, m_passBuffer, m_lensDirtTexture, m_intensity, m_lensDirtIntensity, m_exposure, 0.8f);
    
        m_properties.SetComputeBuffer(HashCache::Get()->_BloomPassParams, m_passBuffer->GetGraphicsID());

        const float blurSize = 4.0f;
        auto shader = m_shader.lock().get();

        for (int i = 0; i < 6; ++i)
        {
            auto fbo = m_blurTextures[i].get();
            m_properties.SetKeywords({ m_passKeywords[1] });                                 // downsample
            GraphicsAPI::BlitInstanced(i * 5 + 0, 1, fbo, shader, m_properties);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            m_properties.SetKeywords({ m_passKeywords[2] });
            GraphicsAPI::BlitInstanced(i * 5 + 1, 1, fbo, shader, m_properties);             // vertical blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            GraphicsAPI::BlitInstanced(i * 5 + 2, 1, fbo, shader, m_properties);             // horizontal blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            GraphicsAPI::BlitInstanced(i * 5 + 3, 1, fbo, shader, m_properties);             // vertical blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            GraphicsAPI::BlitInstanced(i * 5 + 4, 1, fbo, shader, m_properties);            // horizontal blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::BlitInstanced(30, 1, destination, shader, m_properties);
    }
}