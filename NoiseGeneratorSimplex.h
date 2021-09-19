#pragma once
#include <cstdint>

class NoiseGeneratorSimplex
{
public:

    NoiseGeneratorSimplex(uint64_t* random);

    static int fastFloor(double value);
    static double dot(int* p_151604_0_, double p_151604_1_, double p_151604_3_);
    double getValue(double p_151605_1_, double p_151605_3_);
    void add(double* p_151606_1_, double p_151606_2_, double p_151606_4_, int p_151606_6_, int p_151606_7_, double p_151606_8_, double p_151606_10_, double p_151606_12_);

    static constexpr double SQRT_3 = 1.73205080757;
    static constexpr double F2 = 0.5 * (SQRT_3 - 1.0);
    static constexpr double G2 = (3.0 - SQRT_3) / 6.0;

    double xo, yo, zo;

private:
    void initGrad3();
    int grad3[12][3];
    int p[512];
};