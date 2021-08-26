#include "PrecompiledHeader.h"
#include "EngineScreenshot.h"
#include "Rendering/GraphicsAPI.h"
#include <hlslmath.h>

namespace PK::ECS::Engines
{
    using namespace PK::Rendering;

    // Source: https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/
    static void WriteImage(const char* fileName, byte* pixels, uint width, uint height)
    {
        const uint BYTES_PER_PIXEL = 4u;
        const int DATA_OFFSET_OFFSET = 0x000A;
        const int WIDTH_OFFSET = 0x0012;
        const int HEIGHT_OFFSET = 0x0016;
        const int BITS_PER_PIXEL_OFFSET = 0x001C;
        const int HEADER_SIZE = 14;
        const int INFO_HEADER_SIZE = 40;
        const int NO_COMPRESION = 0;
        const int MAX_NUMBER_OF_COLORS = 0;
        const int ALL_COLORS_REQUIRED = 0;

        FILE* outputFile = fopen(fileName, "wb");
        //*****HEADER************//
        const char* BM = "BM";
        fwrite(&BM[0], 1, 1, outputFile);
        fwrite(&BM[1], 1, 1, outputFile);
        int paddedRowSize = (int)(4 * ceil((float)width / 4.0f)) * BYTES_PER_PIXEL;
        uint fileSize = paddedRowSize * height + HEADER_SIZE + INFO_HEADER_SIZE;
        fwrite(&fileSize, 4, 1, outputFile);
        uint reserved = 0x0000;
        fwrite(&reserved, 4, 1, outputFile);
        uint dataOffset = HEADER_SIZE + INFO_HEADER_SIZE;
        fwrite(&dataOffset, 4, 1, outputFile);

        //*******INFO*HEADER******//
        uint infoHeaderSize = INFO_HEADER_SIZE;
        fwrite(&infoHeaderSize, 4, 1, outputFile);
        fwrite(&width, 4, 1, outputFile);
        fwrite(&height, 4, 1, outputFile);
        ushort planes = 1; //always 1
        fwrite(&planes, 2, 1, outputFile);
        ushort bitsPerPixel = BYTES_PER_PIXEL * 8;
        fwrite(&bitsPerPixel, 2, 1, outputFile);
        //write compression
        uint compression = NO_COMPRESION;
        fwrite(&compression, 4, 1, outputFile);
        //write image size(in bytes)
        uint imageSize = width * height * BYTES_PER_PIXEL;
        fwrite(&imageSize, 4, 1, outputFile);
        uint resolutionX = 11811; //300 dpi
        uint resolutionY = 11811; //300 dpi
        fwrite(&resolutionX, 4, 1, outputFile);
        fwrite(&resolutionY, 4, 1, outputFile);
        uint colorsUsed = MAX_NUMBER_OF_COLORS;
        fwrite(&colorsUsed, 4, 1, outputFile);
        uint importantColors = ALL_COLORS_REQUIRED;
        fwrite(&importantColors, 4, 1, outputFile);
        int unpaddedRowSize = width * BYTES_PER_PIXEL;

        for (uint y = 0u; y < height; ++y)
        for (uint x = 0u; x < width; ++x)
        {
            uint index = (x + y * width) * BYTES_PER_PIXEL;
            byte color[4] = { pixels[index + 2], pixels[index + 1], pixels[index + 0], pixels[index + 3] };
            fwrite(color, sizeof(byte), 4, outputFile);
        }

        fclose(outputFile);
    }
    
    void EngineScreenshot::Step(ConsoleCommandToken* token)
    {
        if (!token->isConsumed && token->argument == "take_screenshot")
        {
            m_captureResolution = GraphicsAPI::GetActiveWindowResolution();
            m_accumulatedPixels.clear();
            m_accumulatedPixels.resize(m_captureResolution.x * m_captureResolution.y * 4ul);
            m_captureFrameCount = m_captureFramesRemaining = 8;
            token->isConsumed = true;
        }
    }
    
    void EngineScreenshot::Step(int condition)
    {
        if (m_captureFramesRemaining <= 0)
        {
            return;
        }

        m_captureFramesRemaining--;

        auto resolution = GraphicsAPI::GetActiveWindowResolution();
        auto elementcount = resolution.x * resolution.y * 4ul;

        if (resolution != m_captureResolution)
        {
            m_captureFramesRemaining = 0;
            std::vector<ushort>().swap(m_accumulatedPixels);
            PK_CORE_LOG_WARNING("Capture resolution changed mid capture!");
            return;
        }

        auto accum = m_accumulatedPixels.data();
        auto pixels = PK_CONTIGUOUS_ALLOC(byte, elementcount);
        glReadPixels(0, 0, resolution.x, resolution.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        for (auto i = 0u; i < elementcount; ++i)
        {
            accum[i] += pixels[i];
        }

        if (m_captureFramesRemaining > 0)
        {
            free(pixels);
            return;
        }
        
        for (auto i = 0u; i < elementcount; ++i)
        {
            auto value = accum[i] / m_captureFrameCount;

            if (value > 255)
            {
                value = 255;
            }

            pixels[i] = value;
        }
        

        auto filename = std::string("Screenshot0.bmp");
        auto index = 0;
        
        while (std::filesystem::exists(filename))
        {
            filename = std::string("Screenshot") + std::to_string(++index) + std::string(".bmp");
        }

        WriteImage(filename.c_str(), pixels, resolution.x, resolution.y);
        free(pixels);
        
        PK_CORE_LOG("Screenshot captured: %s", filename);
    }
}