#pragma once

#include "generator.h"
#include "ChunkGenerator.h"
#include <vector>
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <atomic>

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

using ProgressCallback = std::function<void(double)>;
using FoundCallback = std::function<void(std::pair<int, int>)>;
using FinishedCallback = std::function<void(void)>;
using SearchFunc = std::function<void(uint64_t, MCversion, int, int, int, int, int, int, std::vector<FormationBlock>, std::unordered_set<int>*, bool, std::atomic_bool*, ProgressCallback*, FoundCallback)>;

void terrainSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback*, FoundCallback);
void cachedTerrainSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback*, FoundCallback); //twice as fast at the cost of a shitload of memory

void heightmapSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback*, FoundCallback);
void cachedHeightmapSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback*, FoundCallback); //twice as fast at the cost of a shitload of memory

void threadedSearch(SearchFunc func, int numThreads, uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int> biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback*, FoundCallback, FinishedCallback);