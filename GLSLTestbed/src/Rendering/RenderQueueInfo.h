#pragma once
#include "Core/IService.h"
#include "Core/ISingleton.h"
#include <cstdint>
#include <string>

namespace PK::Rendering
{
    class RenderQueueInfo : public PK::Core::IService, public PK::Core::ISingleton<RenderQueueInfo>
    {
        struct RenderQueue
        {
            std::string name;
            uint32_t queueIndex;
        };

        public:
            const RenderQueue Passes[4] =
            {
                { "Default", 0 },
                { "Opaque", 1000 },
                { "Transparent", 2000 },
                { "Overlay", 3000 }
            };

            const uint32_t PassCount = 4;

            uint32_t GetQueueIndex(const std::string& passName)
            {
                for (uint32_t i = 0; i < PassCount; ++i)
                {
                    if (Passes[i].name == passName)
                    {
                        return Passes[i].queueIndex;
                    }
                }

                return 0;
            }
    };
}