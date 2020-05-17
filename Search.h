#pragma once

#include "ChunkGenerator.h"
#include <vector>
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>

class FormationBlock
{
public:
    FormationBlock(int x, int y, int z, int blockid) :
        x_(x),
        y_(y),
        z_(z),
        blockid_(blockid)
    {}

    FormationBlock(const FormationBlock&) = default;

    const int getX() { return x_; }
    const int getY() { return y_; }
    const int getZ() { return z_; }
    const int getBlockId() { return blockid_; }
private:
    int x_, y_, z_, blockid_;

};

using TerrainSearchFunc = std::function <void(int64_t, int, int, int, int, int, int, std::vector<FormationBlock>, std::unordered_set<int>*, bool)>;

void search(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations);
void cachedSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations); //twice as fast at the cost of a shitload of memory
void threadedSearch(TerrainSearchFunc func, int numThreads, int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int> biomes, bool allRotations);