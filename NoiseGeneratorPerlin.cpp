#include "NoiseGeneratorPerlin.h"

NoiseGeneratorPerlin::NoiseGeneratorPerlin(int64_t* p_i45470_1_, int p_i45470_2_)
{
    levels = p_i45470_2_;

    for (int i = 0; i < p_i45470_2_; ++i)
    {
        noiseLevels.emplace_back(p_i45470_1_);//THIS MAY NOT GIVE ORDERING RIGHT????
    }
}

double NoiseGeneratorPerlin::getValue(double p_151601_1_, double p_151601_3_)
{
    double d0 = 0.0;
    double d1 = 1.0;

    for (int i = 0; i < levels; ++i)
    {
        d0 += noiseLevels[i].getValue(p_151601_1_ * d1, p_151601_3_ * d1) / d1;
        d1 /= 2.0;
    }

    return d0;
}

double* NoiseGeneratorPerlin::getRegion(double* p_151599_1_, int p_151599_1_length, double p_151599_2_, double p_151599_4_, int p_151599_6_, int p_151599_7_, double p_151599_8_, double p_151599_10_, double p_151599_12_)
{
    return getRegion(p_151599_1_, p_151599_1_length, p_151599_2_, p_151599_4_, p_151599_6_, p_151599_7_, p_151599_8_, p_151599_10_, p_151599_12_, 0.5);
}

double* NoiseGeneratorPerlin::getRegion(double* p_151600_1_, int p_151600_1_length,  double p_151600_2_, double p_151600_4_, int p_151600_6_, int p_151600_7_, double p_151600_8_, double p_151600_10_, double p_151600_12_, double p_151600_14_)
{
    if (p_151600_1_ != nullptr && p_151600_1_length >= p_151600_6_ * p_151600_7_)
    {
        for (int i = 0; i < p_151600_1_length; ++i)
        {
            p_151600_1_[i] = 0.0;
        }
    }
    else
    {
        p_151600_1_ = new double[p_151600_6_ * p_151600_7_];
        p_151600_1_length = p_151600_6_ * p_151600_7_;
        for (int i = 0; i < p_151600_1_length; ++i)
        {
            p_151600_1_[i] = 0.0;
        }
    }

    double d1 = 1.0;
    double d0 = 1.0;

    for (int j = 0; j < levels; ++j)
    {
        noiseLevels[j].add(p_151600_1_, p_151600_2_, p_151600_4_, p_151600_6_, p_151600_7_, p_151600_8_ * d0 * d1, p_151600_10_ * d0 * d1, 0.55 / d1);
        d0 *= p_151600_12_;
        d1 *= p_151600_14_;
    }

    return p_151600_1_;
}