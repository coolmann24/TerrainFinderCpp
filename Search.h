#pragma once

#include "ChunkGenerator.h"
#include <vector>
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>

class FormationBlock //for heightmap formation, air=0, terrain=1
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

void terrainSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations);
void cachedTerrainSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations); //twice as fast at the cost of a shitload of memory
void threadedTerrainSearch(TerrainSearchFunc func, int numThreads, int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int> biomes, bool allRotations);


using HeightmapSearchFunc = std::function <void(int64_t, int, int, int, int, int, int, std::vector<FormationBlock>, int, bool)>;

void heightmapSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, int biome, bool allRotations);
void cachedHeightmapSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, int biome, bool allRotations); //twice as fast at the cost of a shitload of memory
void threadedHeightmapSearch(HeightmapSearchFunc func, int numThreads, int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, int biome, bool allRotations);