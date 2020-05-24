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
#include <string>

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
    static const constexpr int PODZOL = 13;
    static const constexpr int COARSE_DIRT = 14;
    static const constexpr int RED_SAND = 15;
    static const constexpr int NETHERRACK = 16;
    static const constexpr int SOUL_SAND = 17;
    static const constexpr int END_STONE = 18;
    static const constexpr int RED_SANDSTONE = 19;

    //"ternary surface" ids
    static const constexpr int AIR_T = 0;
    static const constexpr int PODZOL_T = 1;
    static const constexpr int GRAVEL_T = 2;
    static const constexpr int GRASS_T = 3;
    static const constexpr int DIRT_T = 4;
    static const constexpr int STONE_T = 5;
    static const constexpr int COARSE_DIRT_T = 6;
    static const constexpr int SAND_T = 7;
    static const constexpr int GRASS_SAND_UNDERWATER_T = 8;
    static const constexpr int SAND_SAND_UNDERWATER_T = 9;
    static const constexpr int MESA_T = 10;
    static const constexpr int MYCELIUM_T = 11;
    static const constexpr int NETHER_T = 12;
    static const constexpr int SOUL_SAND_T = 13;
    static const constexpr int END_T = 14;
    static const constexpr int CRIMSON_NYLIUM_T = 15;
    static const constexpr int WARPED_NYLIUM_T = 16;
    static const constexpr int ICE_SPIKES_T = 17;

    //"surface builder" ids
    static const constexpr int MOUNTAIN_S = 0; //mountains used to be hills in 1.12
    static const constexpr int SHATTERED_SAVANNA_S = 1;
    static const constexpr int GRAVELLY_MOUNTAIN_S = 2;
    static const constexpr int GIANT_TREE_TAIGA_S = 3;
    static const constexpr int SWAMP_S = 4;
    static const constexpr int MESA_S = 5;
    static const constexpr int WOODED_MESA_S = 6;
    static const constexpr int ERODED_MESA_S = 7;
    static const constexpr int FROZEN_OCEAN_S = 8;
    static const constexpr int NETHER_S = 9;
    static const constexpr int NETHER_FOREST_S = 10;
    static const constexpr int SOUL_SAND_VALLEY_S = 11;
    static const constexpr int BASALT_DELTAS_S = 12;

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

    int top_block_, filler_block_, underwater_block_;
    bool amplified_;
    std::unique_ptr<NoiseGeneratorOctaves> minLimitPerlinNoise;
    std::unique_ptr<NoiseGeneratorOctaves> maxLimitPerlinNoise;
    std::unique_ptr<NoiseGeneratorOctaves> mainPerlinNoise;
    std::unique_ptr<NoiseGeneratorPerlin> surfaceNoise;
    std::unique_ptr<NoiseGeneratorPerlin> mesaPillarNoise;
    std::unique_ptr<NoiseGeneratorPerlin> mesaPillarRoofNoise;
    std::unique_ptr<NoiseGeneratorPerlin> grassColorNoise;
    std::unique_ptr<NoiseGeneratorOctaves> scaleNoise;
    std::unique_ptr<NoiseGeneratorOctaves> depthNoise;
    std::unique_ptr<NoiseGeneratorOctaves> forestNoise;
    double heightMap[825];
    float biomeWeights[25];
    int biomesForGeneration1[100];
    int biomesForGeneration2[256];

    LayerStack stack_;
    std::unordered_map<int, std::pair<float, float>> biome_to_base_and_variation_;

    std::unordered_map<int, int> biome_to_ternary_;
    std::unordered_map<int, std::tuple<int, int, int>> ternary_to_blocktypes_; //top, filler, and underwater blocks

    void registerBaseAndVariation();
    float getBaseHeight(int biome);
    float getHeightVariation(int biome);

    void registerTernaryBlocks();
    void registerTernaryIds();

    using SurfaceBuilderAdditionsFunc = std::function<void(int64_t*, ChunkData&, int, int, double)>;
    std::unordered_map<int, int> biome_to_surface_builder_;
    std::unordered_map<int, SurfaceBuilderAdditionsFunc> surface_builder_to_additions_func_;
    //1.12 had a 'genTerrainBlocks' function for additional biome generation, but 1.16- has 'surface builders', of which most use the 'default' which generateBiomeTerrain implements
    //mesa still overrides this generateBiomeTerrain completely, frozen ocean too but dont support now

    void registerSurfaceBuilderIds();
    void registerSurfaceBuilderFuncs(MCversion version);

    std::unordered_set<int> no_def_surface_building_;//only for mesa

    MCversion version_;

    
public:
    ChunkGenerator(int64_t world_seed, MCversion version);
    void provideChunk(int x, int z, ChunkData& chunk, std::unordered_set<int>* biomes = nullptr); //NOT THREAD SAFE

private:
    void setBlocksInChunk(int x, int z, ChunkData& primer);
    void generateHeightmap(int p_185978_1_, int p_185978_2_, int p_185978_3_);
    void generateBiomeTerrain112(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal);
    void defaultSurfaceBuild113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal);
    void buildBedrock113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal);
    void buildBedrock114(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal);
    void replaceBiomeBlocks(int64_t* rand, int x, int z, ChunkData& primer);

private:

    void HillsMountainsSB(int64_t*, ChunkData&, int, int, double, bool);//1.12 called hills, mountains 1.13+
    void MesaSB112(int64_t*, ChunkData&, int, int, double, bool, bool);//there is slight variation in mesa generation between 1.12/1.13+
    void MesaDefSB113(int64_t*, ChunkData&, int, int, double);
    void MesaWoodedSB113(int64_t*, ChunkData&, int, int, double);
    void MesaErodedSB113(int64_t*, ChunkData&, int, int, double);
    void SwampSB(int64_t*, ChunkData&, int, int, double);
    void TaigaSB(int64_t*, ChunkData&, int, int, double);
    void SavannaMutatedSB(int64_t*, ChunkData&, int, int, double);

};
