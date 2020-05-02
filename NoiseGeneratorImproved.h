#pragma once
#include <cstdint>
#include <iostream>

class NoiseGeneratorImproved {
public:
    NoiseGeneratorImproved(int64_t*);
    double lerp(double p_76311_1_, double p_76311_3_, double p_76311_5_);
    double grad2(int p_76309_1_, double p_76309_2_, double p_76309_4_);
    double grad(int p_76310_1_, double p_76310_2_, double p_76310_4_, double p_76310_6_);
    void populateNoiseArray(double* noiseArray, double xOffset, double yOffset, double zOffset, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale, double noiseScale);

    double xCoord;
    double yCoord;
    double zCoord;

private:

    int permutations[512];
    
    const static double GRAD_X[];
    const static double GRAD_Y[];
    const static double GRAD_Z[];
    const static double GRAD_2X[];
    const static double GRAD_2Z[];
};