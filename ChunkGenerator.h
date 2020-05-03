#pragma once

#include "NoiseGeneratorOctaves.h"
#include "NoiseGeneratorPerlin.h"
#include "ChunkData.h"
#include "javarnd.h"
#include "generator.h"
#include "layers.h"
#include <tuple>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>

class ChunkGenerator
{
public:
    static const constexpr int AIR = 0;
    static const constexpr int STONE = 1;
    static const constexpr int SAND = 2;
    static const constexpr int GRASS = 3;
    static const constexpr int DIRT = 4;
    static const constexpr int BEDROCK = 5;
    static const constexpr int WATER = 6;
    static const constexpr int GRAVEL = 7;
    static const constexpr int HARDCLAY = 8;
    static const constexpr int MYCELIUM = 9;
    static const constexpr int SNOW_BLOCK = 10;
    static const constexpr int WATER_LILY = 11;
    static const constexpr int SANDSTONE = 12;

    static const constexpr int BEACH = 0;
    static const constexpr int DESERT = 1;
    static const constexpr int END = 2;
    static const constexpr int FOREST = 3;
    static const constexpr int HELL = 4;
    static const constexpr int HILLSNORMAL = 5;
    static const constexpr int JUNGLE = 6;
    static const constexpr int MESANORMAL = 7;
    static const constexpr int MUSHROOMISLAND = 8;
    static const constexpr int OCEAN = 9;
    static const constexpr int PLAINS = 10;
    static const constexpr int RIVER = 11;
    static const constexpr int SAVANNA = 12;
    static const constexpr int SNOW = 13;
    static const constexpr int STONEBEACH = 14;
    static const constexpr int SWAMP = 15;
    static const constexpr int TAIGANORMAL = 16;
    static const constexpr int VOID = 17;

    static const constexpr int FORESTMUTATED = 18;
    static const constexpr int SAVANNAMUTATED = 19;

    static const constexpr int HILLSEXTRATREES = 20;
    static const constexpr int HILLSMUTATED = 21;

    static const constexpr int MESABRYCE = 22;
    static const constexpr int MESAFOREST = 23;
    static const constexpr int MESABRYCEFOREST = 24;

    static const constexpr int TAIGAMEGA = 25;

private:
    static const constexpr float depthNoiseScaleX = 200.0;
    static const constexpr float depthNoiseScaleZ = 200.0;
    static const constexpr float depthNoiseScaleExponent = 0.5;
    static const constexpr float mainNoiseScaleX = 80.0;
    static const constexpr float mainNoiseScaleY = 160.0;
    static const constexpr float mainNoiseScaleZ = 80.0;
    static const constexpr float coordinateScale = 684.412F;
    static const constexpr float heightScale = 684.412F;
    static const constexpr float biomeDepthOffset = 0.0;
    static const constexpr float biomeDepthWeight = 1.0;
    static const constexpr float biomeScaleOffset = 0.0;
    static const constexpr float biomeScaleWeight = 1.0;
    static const constexpr float baseSize = 8.5F;
    static const constexpr float stretchY = 12.0F;
    static const constexpr float upperLimitScale = 512.0F;
    static const constexpr float lowerLimitScale = 512.0F;

    float biome_base_height_, biome_height_variation_;
    int top_block_, filler_block_;
    bool amplified_;
    NoiseGeneratorOctaves minLimitPerlinNoise;
    NoiseGeneratorOctaves maxLimitPerlinNoise;
    NoiseGeneratorOctaves mainPerlinNoise;
    NoiseGeneratorPerlin surfaceNoise;
    std::unique_ptr<NoiseGeneratorPerlin> mesaPillarNoise;
    std::unique_ptr<NoiseGeneratorPerlin> mesaPillarRoofNoise;
    std::unique_ptr<NoiseGeneratorPerlin> grassColorNoise;
    NoiseGeneratorOctaves scaleNoise;
    NoiseGeneratorOctaves depthNoise;
    NoiseGeneratorOctaves forestNoise;
    double heightMap[825];
    float biomeWeights[25];

    LayerStack stack_;
    std::unordered_map<int, std::pair<float, float>> biome_to_base_and_variation_;
    std::unordered_map<int, std::pair<int, int>> biome_to_top_and_filler_;
    std::unordered_map<int, int> biome_id_to_biome_type_;

    void registerBaseAndVariation();
    void registerTopAndFiller();
    void registerBiomeIdTypeMappings();

    using GenTerrainBlocksFunc = std::function<void(int64_t*, ChunkData&, int, int, double)>;
    std::unordered_map<int, GenTerrainBlocksFunc> biome_type_to_gen_terrain_blocks_;

    void registerGenTerrainFuncs();

    std::unordered_set<int> dont_generate_biome_terrain;//only for mesa

    
public:
    ChunkGenerator(int64_t world_seed, int64_t* randprimedseed);
    void provideChunk(int x, int z, ChunkData& chunk, std::unordered_set<int>* biomes = nullptr);

private:
    void setBlocksInChunk(int x, int z, ChunkData& primer);
    void generateHeightmap(int p_185978_1_, int p_185978_2_, int p_185978_3_);
    void generateBiomeTerrain(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal);
    void replaceBiomeBlocks(int64_t* rand, int x, int z, ChunkData& primer, int biome);

private:

    void BeachGTB(int64_t*, ChunkData&, int, int, double);
    void DesertGTB(int64_t*, ChunkData&, int, int, double);
    void EndGTB(int64_t*, ChunkData&, int, int, double);
    void ForestGTB(int64_t*, ChunkData&, int, int, double);
    void HellGTB(int64_t*, ChunkData&, int, int, double);
    void HillsGTB(int64_t*, ChunkData&, int, int, double, bool, bool);
    void JungleGTB(int64_t*, ChunkData&, int, int, double);
    void MesaGTB(int64_t*, ChunkData&, int, int, double, bool, bool);
    void MushroomIslandGTB(int64_t*, ChunkData&, int, int, double);
    void OceanGTB(int64_t*, ChunkData&, int, int, double);
    void PlainsGTB(int64_t*, ChunkData&, int, int, double);
    void RiverGTB(int64_t*, ChunkData&, int, int, double);
    void SavannaGTB(int64_t*, ChunkData&, int, int, double);
    void SnowGTB(int64_t*, ChunkData&, int, int, double);
    void StoneBeachGTB(int64_t*, ChunkData&, int, int, double);
    void SwampGTB(int64_t*, ChunkData&, int, int, double);
    void TaigaGTB(int64_t*, ChunkData&, int, int, double, bool);
    void VoidGTB(int64_t*, ChunkData&, int, int, double);

    void ForestMutatedGTB(int64_t*, ChunkData&, int, int, double);
    void SavannaMutatedGTB(int64_t*, ChunkData&, int, int, double);

};
