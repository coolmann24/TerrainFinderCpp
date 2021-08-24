#include "NoiseGeneratorOctaves.h"

NoiseGeneratorOctaves::NoiseGeneratorOctaves(uint64_t* seed, int octavesIn)
{
    octaves = octavesIn;

    for (int i = 0; i < octavesIn; ++i)
    {
        generatorCollection.emplace_back(seed);//AGAIN MAY NOT WORK RIGHT
    }
}

double* NoiseGeneratorOctaves::generateNoiseOctaves(double* noiseArray, int noiseArrayLength, int xOffset, int yOffset, int zOffset, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale)
{
    if (noiseArray == nullptr)
    {
        noiseArray = new double[xSize * ySize * zSize];
        noiseArrayLength = xSize * ySize * zSize;
        for (int i = 0; i < noiseArrayLength; ++i)
        {
            noiseArray[i] = 0.0;
        }
    }
    else
    {
        for (int i = 0; i < noiseArrayLength; ++i)
        {
            noiseArray[i] = 0.0;
        }
    }

    double d3 = 1.0;

    for (int j = 0; j < octaves; ++j)
    {
        double d0 = (double)xOffset * d3 * xScale;
        double d1 = (double)yOffset * d3 * yScale;
        double d2 = (double)zOffset * d3 * zScale;
        int64_t k = (int64_t)(d0);
        int64_t l = (int64_t)(d2);
        d0 = d0 - (double)k;
        d2 = d2 - (double)l;
        k = k % 16777216L;
        l = l % 16777216L;
        d0 = d0 + (double)k;
        d2 = d2 + (double)l;
        generatorCollection[j].populateNoiseArray(noiseArray, d0, d1, d2, xSize, ySize, zSize, xScale * d3, yScale * d3, zScale * d3, d3);
        d3 /= 2.0;
    }

    return noiseArray;
}

double* NoiseGeneratorOctaves::generateNoiseOctaves(double* noiseArray, int noiseArrayLength, int xOffset, int zOffset, int xSize, int zSize, double xScale, double zScale, double p_76305_10_)
{
    return generateNoiseOctaves(noiseArray, noiseArrayLength, xOffset, 10, zOffset, xSize, 1, zSize, xScale, 1.0, zScale);
}