#include "PrecompiledHeader.h"
#include "FilterAO.h"
#include "Rendering/GraphicsAPI.h"
#include "Utilities/HashCache.h"
#include "FilterFlowGraph.h"

namespace PK::Rendering::PostProcessing
{
    const static char CELL_VISITED = 1 << 5;
    const static int OFFSETSX[4] = { 1,  0, -1, 0 };
    const static int OFFSETSY[4] = { 0, -1,  0, 1 };

    static void GenerateMaze(uint32_t w, uint32_t h, uint32_t pw, char* texture)
    {
        auto tw = w * pw;
        auto th = h * pw;
    
        auto maze = reinterpret_cast<char*>(calloc(w * h, sizeof(char)));
        auto stack = reinterpret_cast<uint32_t*>(malloc(w * h * sizeof(uint32_t)));
    
        auto offs = pw - 1u;
        auto stackSize = 1u;
        auto ncells = w * h - 1u;
        maze[0] = CELL_VISITED;
        stack[0] = 0u;
    
        while (ncells > 0)
        {
            uint32_t i = stack[stackSize - 1];
            uint32_t n = 0u;
            uint32_t cx = i % w;
            uint32_t cy = i / w;
    
            auto dir = 0xFF;
    
            for (uint32_t j = rand(), k = j + 4; j < k; ++j)
            {
                int nx = cx + OFFSETSX[j % 4];
                int ny = cy + OFFSETSY[j % 4];
                n = nx + ny * w;
    
                if (nx >= 0 && nx < w && ny >= 0 && ny < h && (maze[n] & CELL_VISITED) == 0)
                {
                    dir = j % 4;
                    break;
                }
            }
    
            if (dir == 0xFF)
            {
                stackSize--;
                continue;
            }
    
            maze[n] |= CELL_VISITED | (1 << ((dir + 2) % 4));
            maze[i] |= (1 << dir);
    
            stack[stackSize++] = n;
            ncells--;
        }
    
        for (auto x = 0u; x < w; ++x)
        for (auto y = 0u; y < h; ++y)
        {
            auto cell = maze[x + y * w];
    
            auto tx = x * pw;
            auto ty = y * pw;
    
            if ((cell & (1 << 3)) == 0 && tx > 0)
            {
                texture[tx - 1 + (ty + offs) * tw] = 255;
            }
    
            for (auto i = 0; i < pw; ++i)
            {
                if ((cell & (1 << 0)) == 0)
                {
                    texture[tx + offs + (ty + i) * tw] = 255;
                }
    
                if ((cell & (1 << 3)) == 0)
                {
                    texture[tx + i + (ty + offs) * tw] = 255;
                }
            }
        }
    
        free(maze);
        free(stack);
    }

    FilterFlowGraph::FilterFlowGraph(AssetDatabase* assetDatabase, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_FlowRender"))
    {
        auto gw = 64;
        auto gh = 32;
        auto pw = 8;
        auto tw = gw * pw;
        auto th = gh * pw;

        auto mazeTexture = reinterpret_cast<char*>(calloc(tw * th, sizeof(char)));

        GenerateMaze(gw, gh, pw, mazeTexture);

        m_shaderFlowIterate = assetDatabase->Find<Shader>("CS_FlowIterate");
        m_shaderBoidUpdate = assetDatabase->Find<Shader>("CS_FlowBoidUpdate");
        m_shaderDrawBoids = assetDatabase->Find<Shader>("SH_VS_FlowBoids");
        
        auto descriptor = RenderTextureDescriptor();
        descriptor.dimension = GL_TEXTURE_2D;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_R32UI, GL_R8 };
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        descriptor.resolution.x = tw;
        descriptor.resolution.y = th;
        descriptor.resolution.z = 1;
        m_renderTarget = CreateRef<RenderTexture>(descriptor);
        m_renderTarget->GetColorBuffer(1)->SetData(mazeTexture, tw * th * sizeof(char), 0);

        m_boidsBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::FLOAT4, "POSITION"} }), 1024, true, GL_NONE);

        m_properties.SetTexture(StringHashID::StringToID("pk_Maze"), m_renderTarget->GetColorBuffer(1)->GetGraphicsID());
        m_properties.SetImage(StringHashID::StringToID("pk_FlowTexture"), m_renderTarget->GetColorBuffer(0)->GetImageBindDescriptor(GL_READ_WRITE, 0, 0, false));
        m_properties.SetComputeBuffer(StringHashID::StringToID("pk_Boids"), m_boidsBuffer->GetGraphicsID());
    
        free(mazeTexture);
    }

    void FilterFlowGraph::Execute(const RenderTexture* destination)
    {
        const uint flowIterationsPerFrame = 4u;

        for (auto i = 0u; i < flowIterationsPerFrame; ++i)
        {
            GraphicsAPI::DispatchCompute(m_shaderFlowIterate, { m_renderTarget->GetWidth() / 32, m_renderTarget->GetHeight() / 32, 1 }, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        GraphicsAPI::DispatchCompute(m_shaderBoidUpdate, { 32, 1, 1 }, m_properties, GL_SHADER_STORAGE_BARRIER_BIT);
        GraphicsAPI::Blit(destination, m_shader, m_properties);
        GraphicsAPI::BlitInstanced(0 ,1024, m_shaderDrawBoids, m_properties);
    }
}