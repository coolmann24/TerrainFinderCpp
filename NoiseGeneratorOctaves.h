#pragma once
#include <cstdint>
#include <vector>
#include "NoiseGeneratorImproved.h"

class NoiseGeneratorOctaves
{
public:
	NoiseGeneratorOctaves(uint64_t* seed, int octavesIn);
	double* generateNoiseOctaves(double* noiseArray, int noiseArrayLength, int xOffset, int yOffset, int zOffset, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale);
	double* generateNoiseOctaves(double* noiseArray, int noiseArrayLength, int xOffset, int zOffset, int xSize, int zSize, double xScale, double zScale, double p_76305_10_);
private:
	std::vector<NoiseGeneratorImproved> generatorCollection;
	int octaves;
};