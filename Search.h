#pragma once

#include "ChunkGenerator.h"
#include <vector>
#include <iostream>

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

void search(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int>* biomes, bool allRotations = true);
//void loopSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int>* biomes, bool allRotations = true) {}
//void cachedSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int>* biomes, bool allRotations = true) {}
void threadedSearch(int64_t seed, int numThreads, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int>* biomes, bool allRotations = true);