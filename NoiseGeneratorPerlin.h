#pragma once

#include "NoiseGeneratorSimplex.h"
#include <vector>

class NoiseGeneratorPerlin
{
private:
	std::vector<NoiseGeneratorSimplex> noiseLevels;
	int levels;

public:
	NoiseGeneratorPerlin(uint64_t* p_i45470_1_, int p_i45470_2_);
	double getValue(double p_151601_1_, double p_151601_3_);
	double* getRegion(double* p_151599_1_, int p_151599_1_length, double p_151599_2_, double p_151599_4_, int p_151599_6_, int p_151599_7_, double p_151599_8_, double p_151599_10_, double p_151599_12_);
	double* getRegion(double* p_151600_1_, int p_151600_1_length, double p_151600_2_, double p_151600_4_, int p_151600_6_, int p_151600_7_, double p_151600_8_, double p_151600_10_, double p_151600_12_, double p_151600_14_);
};
