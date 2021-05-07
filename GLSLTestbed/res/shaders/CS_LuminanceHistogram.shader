#version 460
#extension GL_ARB_bindless_texture : require
#multi_compile PASS_HISTOGRAM PASS_AVG

#pragma PROGRAM_COMPUTE
#include includes/PKCommon.glsl
#include includes/Tonemapping.glsl

// Source: http://www.alextardif.com/HistogramLuminance.html
shared uint HistogramShared[NUM_HISTOGRAM_BINS];

uint HDRToHistogramBin(float3 hdrColor)
{
    float luminance = dot(pk_Luminance.rgb, hdrColor);

    if (luminance < EPSILON)
    {
        return 0;
    }

    float logLuminance = saturate((log2(luminance) - LOG_LUMINANCE_MIN) * LOG_LUMINANCE_INV_RANGE);

    return uint(logLuminance * 254.0 + 1.0);
}

layout(local_size_x = HISTOGRAM_THREAD_COUNT, local_size_y = HISTOGRAM_THREAD_COUNT, local_size_z = 1) in;
void main()
{
    #if defined(PASS_HISTOGRAM)
        HistogramShared[gl_LocalInvocationIndex] = 0;
        barrier();
    
        if (gl_GlobalInvocationID.x < pk_ScreenParams.x && gl_GlobalInvocationID.y < pk_ScreenParams.y)
        {
            float3 hdrColor = texelFetch(pk_HDRScreenTex, int2(gl_GlobalInvocationID.xy), 0).xyz;
            uint binIndex = HDRToHistogramBin(hdrColor);
            atomicAdd(HistogramShared[binIndex], 1);
        }
    
        barrier();
        atomicAdd(PK_BUFFER_DATA(pk_Histogram, gl_LocalInvocationIndex), HistogramShared[gl_LocalInvocationIndex]);
    #else
        uint countForThisBin = PK_BUFFER_DATA(pk_Histogram, gl_LocalInvocationIndex);
        HistogramShared[gl_LocalInvocationIndex] = countForThisBin * gl_LocalInvocationIndex;

        PK_BUFFER_DATA(pk_Histogram, gl_LocalInvocationIndex) = 0;

        barrier();

        #pragma unroll
        for (uint histogramSampleIndex = (NUM_HISTOGRAM_BINS >> 1); histogramSampleIndex > 0; histogramSampleIndex >>= 1)
        {
            if (gl_LocalInvocationIndex < histogramSampleIndex)
            {
                HistogramShared[gl_LocalInvocationIndex] += HistogramShared[gl_LocalInvocationIndex + histogramSampleIndex];
            }

            barrier();
        }

        if (gl_LocalInvocationIndex == 0)
        {
            float numpx = pk_ScreenParams.x * pk_ScreenParams.y;
            float weightedLogAverage = (HistogramShared[0] / max(numpx - countForThisBin, 1.0)) - 1.0;
            float weightedAverageLuminance = exp2((weightedLogAverage / 254.0) * LOG_LUMINANCE_RANGE + LOG_LUMINANCE_MIN);

            float EV100 = computeEV100FromAvgLuminance(weightedAverageLuminance);

            float targetExposure = TARGET_EXPOSURE * convertEV100ToExposure(EV100);
            float exposure = GetAutoExposure();
            exposure = lerp(exposure, targetExposure, pk_DeltaTime.x * EXPOSURE_ADJUST_SPEED);

            SetAutoExposure(exposure);
        }
    #endif
}