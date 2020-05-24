#include "ChunkGenerator.h"
#include <cmath>

ChunkGenerator::ChunkGenerator(int64_t world_seed, MCversion version)
{
    version_ = version;

    int64_t seed = world_seed;
    int64_t seedcopy = world_seed;
    setSeed(&seed);

    minLimitPerlinNoise = std::make_unique<NoiseGeneratorOctaves>(&seed, 16);
    maxLimitPerlinNoise = std::make_unique<NoiseGeneratorOctaves>(&seed, 16);
    mainPerlinNoise = std::make_unique<NoiseGeneratorOctaves>(&seed, 8);
    surfaceNoise = std::make_unique<NoiseGeneratorPerlin>(&seed, 4);
    scaleNoise = std::make_unique<NoiseGeneratorOctaves>(&seed, 10);
    depthNoise = std::make_unique<NoiseGeneratorOctaves>(&seed, 16);
    forestNoise = std::make_unique<NoiseGeneratorOctaves>(&seed, 8);

    for (int i = -2; i <= 2; ++i)
    {
        for (int j = -2; j <= 2; ++j)
        {
            float f = 10.0F / sqrt((float)(i * i + j * j) + 0.2F);
            biomeWeights[i + 2 + (j + 2) * 5] = f;
        }
    }
    amplified_ = false;

    top_block_ = GRASS;
    filler_block_ = DIRT;
    underwater_block_ = GRAVEL;

    setSeed(&seedcopy);
    mesaPillarNoise = std::make_unique<NoiseGeneratorPerlin>(&seedcopy, 4);
    mesaPillarRoofNoise = std::make_unique<NoiseGeneratorPerlin>(&seedcopy, 1);

    int64_t grassNoiseSeed = 2345LL;
    setSeed(&grassNoiseSeed);
    grassColorNoise = std::make_unique<NoiseGeneratorPerlin>(&grassNoiseSeed, 1);

    stack_ = setupGenerator(version);
    applySeed(&stack_, world_seed);

    registerBaseAndVariation();
    registerTernaryBlocks();
    registerTernaryIds();
    registerSurfaceBuilderIds();
    registerSurfaceBuilderFuncs(version);

    no_def_surface_building_.insert(MESA_S);
    no_def_surface_building_.insert(ERODED_MESA_S);
    no_def_surface_building_.insert(WOODED_MESA_S);
    no_def_surface_building_.insert(FROZEN_OCEAN_S);//still unsupported
}

void ChunkGenerator::provideChunk(int x, int z, ChunkData& chunk, std::unordered_set<int>* biomes)
{
    int* map1 = allocCache(&stack_.layers[L_RIVER_MIX_4], 10, 10);//10x10??
    genArea(&stack_.layers[L_RIVER_MIX_4], map1, x * 4 - 2, z * 4 - 2, 10, 10);
    for (int i = 0; i < 100; i++)
        biomesForGeneration1[i] = map1[i];

    int* map2 = allocCache(&stack_.layers[L_VORONOI_ZOOM_1], 16, 16);
    genArea(&stack_.layers[L_VORONOI_ZOOM_1], map2, x*16, z*16, 16, 16);
    for (int i = 0; i < 256; i++)
        biomesForGeneration2[i] = map2[i];

    if (biomes)
    {
        if (std::end(*biomes) == biomes->find(biomesForGeneration2[0])) //only check one block in chunk, could check every one but the biome probably still doesnt exist and thats just slower
            return;
    }

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            for (int k = 0; k < 256; k++)
                chunk.setBlock(i, k, j, ChunkGenerator::AIR);

    /*for (int i = 15; i >= 0; i--)
    {
        for (int j = 0; j < 16; j++)
        {
            std::cout << biomesForGeneration2[j*16+i] << " ";
        }
        std::cout << std::endl;
    }*/

    int64_t rand = (int64_t)x * 341873128712L + (int64_t)z * 132897987541L;
    setSeed(&rand);
    setBlocksInChunk(x, z, chunk);
    replaceBiomeBlocks(&rand, x, z, chunk);

    free(map1);
    free(map2);
}

static double clampedLerp(double lowerBnd, double upperBnd, double slide)
{
    if (slide < 0.0)
    {
        return lowerBnd;
    }
    else
    {
        return slide > 1.0 ? upperBnd : lowerBnd + (upperBnd - lowerBnd) * slide;
    }
}

void ChunkGenerator::setBlocksInChunk(int x, int z, ChunkData& primer)
{
    generateHeightmap(x * 4, 0, z * 4);

    for (int i = 0; i < 4; ++i)
    {
        int j = i * 5;
        int k = (i + 1) * 5;

        for (int l = 0; l < 4; ++l)
        {
            int i1 = (j + l) * 33;
            int j1 = (j + l + 1) * 33;
            int k1 = (k + l) * 33;
            int l1 = (k + l + 1) * 33;

            for (int i2 = 0; i2 < 32; ++i2)
            {
                double d0 = 0.125;
                double d1 = heightMap[i1 + i2];
                double d2 = heightMap[j1 + i2];
                double d3 = heightMap[k1 + i2];
                double d4 = heightMap[l1 + i2];
                double d5 = (heightMap[i1 + i2 + 1] - d1) * 0.125;
                double d6 = (heightMap[j1 + i2 + 1] - d2) * 0.125;
                double d7 = (heightMap[k1 + i2 + 1] - d3) * 0.125;
                double d8 = (heightMap[l1 + i2 + 1] - d4) * 0.125;

                for (int j2 = 0; j2 < 8; ++j2)
                {
                    double d9 = 0.25;
                    double d10 = d1;
                    double d11 = d2;
                    double d12 = (d3 - d1) * 0.25;
                    double d13 = (d4 - d2) * 0.25;

                    for (int k2 = 0; k2 < 4; ++k2)
                    {
                        double d14 = 0.25;
                        double d16 = (d11 - d10) * 0.25;
                        double lvt_45_1_ = d10 - d16;

                        for (int l2 = 0; l2 < 4; ++l2)
                        {
                            //if (i * 4 + k2 == 2 && l * 4 + l2 == 0 && i2 * 8 + j2 == 6)
                            //    i = i;
                            if ((lvt_45_1_ += d16) > 0.0)
                            {
                                
                                primer.setBlock(i * 4 + k2, i2 * 8 + j2, l * 4 + l2, STONE);
                            }
                            else if (i2 * 8 + j2 < 63)//sealevel
                            {
                                primer.setBlock(i * 4 + k2, i2 * 8 + j2, l * 4 + l2, WATER);
                            }
                        }

                        d10 += d12;
                        d11 += d13;
                    }

                    d1 += d5;
                    d2 += d6;
                    d3 += d7;
                    d4 += d8;
                }
            }
        }
    }
}

void ChunkGenerator::generateHeightmap(int p_185978_1_, int p_185978_2_, int p_185978_3_)
{
    double* depthRegion = depthNoise->generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_3_, 5, 5, depthNoiseScaleX, depthNoiseScaleZ, depthNoiseScaleExponent);
    float f = coordinateScale;
    float f1 = heightScale;
    double* mainNoiseRegion = mainPerlinNoise->generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_2_, p_185978_3_, 5, 33, 5, (double)(f / mainNoiseScaleX), (double)(f1 / mainNoiseScaleY), (double)(f / mainNoiseScaleZ));
    double* minLimitRegion = minLimitPerlinNoise->generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_2_, p_185978_3_, 5, 33, 5, (double)f, (double)f1, (double)f);
    double* maxLimitRegion = maxLimitPerlinNoise->generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_2_, p_185978_3_, 5, 33, 5, (double)f, (double)f1, (double)f);
    int i = 0;
    int j = 0;

    for (int k = 0; k < 5; ++k)
    {
        for (int l = 0; l < 5; ++l)
        {
            float f2 = 0.0F;
            float f3 = 0.0F;
            float f4 = 0.0F;
            int i1 = 2;
            int biome = biomesForGeneration1[k + 2 + (l + 2) * 10];

            for (int j1 = -2; j1 <= 2; ++j1)
            {
                for (int k1 = -2; k1 <= 2; ++k1)
                {
                    int biome1 = biomesForGeneration1[k + j1 + 2 + (l + k1 + 2) * 10];
                    float f5 = biomeDepthOffset + getBaseHeight(biome1) * biomeDepthWeight;
                    float f6 = biomeScaleOffset + getHeightVariation(biome1) * biomeScaleWeight;

                    if (amplified_ && f5 > 0.0F)
                    {
                        f5 = 1.0F + f5 * 2.0F;
                        f6 = 1.0F + f6 * 4.0F;
                    }

                    float f7 = biomeWeights[j1 + 2 + (k1 + 2) * 5] / (f5 + 2.0F);

                    if (getBaseHeight(biome1) > getBaseHeight(biome))
                    {
                        f7 /= 2.0F;
                    }

                    f2 += f6 * f7;
                    f3 += f5 * f7;
                    f4 += f7;
                }
            }

            f2 = f2 / f4;
            f3 = f3 / f4;
            f2 = f2 * 0.9F + 0.1F;
            f3 = (f3 * 4.0F - 1.0F) / 8.0F;
            double d7 = depthRegion[j] / 8000.0;

            if (d7 < 0.0)
            {
                d7 = -d7 * 0.3;
            }

            d7 = d7 * 3.0 - 2.0;

            if (d7 < 0.0)
            {
                d7 = d7 / 2.0;

                if (d7 < -1.0)
                {
                    d7 = -1.0;
                }

                d7 = d7 / 1.4;
                d7 = d7 / 2.0;
            }
            else
            {
                if (d7 > 1.0)
                {
                    d7 = 1.0;
                }

                d7 = d7 / 8.0;
            }

            ++j;
            double d8 = (double)f3;
            double d9 = (double)f2;
            d8 = d8 + d7 * 0.2;
            d8 = d8 * (double)baseSize / 8.0;
            double d0 = (double)baseSize + d8 * 4.0;

            for (int l1 = 0; l1 < 33; ++l1)
            {
                double d1 = ((double)l1 - d0) * (double)stretchY * 128.0 / 256.0 / d9;

                if (d1 < 0.0)
                {
                    d1 *= 4.0;
                }

                double d2 = minLimitRegion[i] / (double)lowerLimitScale;
                double d3 = maxLimitRegion[i] / (double)upperLimitScale;
                double d4 = (mainNoiseRegion[i] / 10.0 + 1.0) / 2.0;
                double d5 = clampedLerp(d2, d3, d4) - d1;

                if (l1 > 29)
                {
                    double d6 = (double)((float)(l1 - 29) / 3.0F);
                    d5 = d5 * (1.0 - d6) + -10.0 * d6;
                }

                heightMap[i] = d5;
                ++i;
            }
        }
    }
    delete[] depthRegion;
    delete[] mainNoiseRegion;
    delete[] minLimitRegion;
    delete[] maxLimitRegion;

}

void ChunkGenerator::generateBiomeTerrain112(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
    int i = 63;//sealevel
    int iblockstate = top_block_;//topblock
    int iblockstate1 = filler_block_;//filler
    int j = -1;
    int k = (int)(noiseVal / 3.0 + 3.0 + nextDouble(rand) * 0.25);
    int l = x & 15;
    int i1 = z & 15;
    std::tuple<int, int, int> blockpos$mutableblockpos;

    for (int j1 = 255; j1 >= 0; --j1)
    {
        if (j1 <= nextInt(rand, 5))
        {
            chunkPrimerIn.setBlock(i1, j1, l, BEDROCK);
        }
        else
        {
            int iblockstate2 = chunkPrimerIn.getBlock(i1, j1, l);

            if (iblockstate2 == AIR)
            {
                j = -1;
            }
            else if (iblockstate2 == STONE)
            {
                if (j == -1)
                {
                    if (k <= 0)
                    {
                        iblockstate = AIR;
                        iblockstate1 = STONE;
                    }
                    else if (j1 >= i - 4 && j1 <= i + 1)
                    {
                        iblockstate = top_block_;
                        iblockstate1 = filler_block_;
                    }

                    if (j1 < i && (iblockstate == AIR))
                    {
                        /*if (getFloatTemperature(blockpos$mutableblockpos.setPos(x, j1, z)) < 0.15F)
                        {
                            iblockstate = ICE;
                        }
                        else
                        {
                            iblockstate = WATER;
                        }*/
                        iblockstate = WATER;
                    }

                    j = k;

                    if (j1 >= i - 1)
                    {
                        chunkPrimerIn.setBlock(i1, j1, l, iblockstate);
                    }
                    else if (j1 < i - 7 - k)
                    {
                        iblockstate = AIR;
                        iblockstate1 = STONE;
                        chunkPrimerIn.setBlock(i1, j1, l, GRAVEL);
                    }
                    else
                    {
                        chunkPrimerIn.setBlock(i1, j1, l, iblockstate1);
                    }
                }
                else if (j > 0)
                {
                    --j;
                    chunkPrimerIn.setBlock(i1, j1, l, iblockstate1);

                    if (j == 0 && iblockstate1 == SAND && k > 1)
                    {
                        j = nextInt(rand, 4) + std::max(0, j1 - 63);
                        iblockstate1 = iblockstate1 == RED_SAND ? RED_SANDSTONE : SANDSTONE;
                    }
                }
            }
        }
    }
}

void ChunkGenerator::defaultSurfaceBuild113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
    int l = 63;
    int lv = top_block_;
    int lv2 = filler_block_;
    int m = -1;
    int n = (int)(noiseVal / 3.0 + 3.0 + nextDouble(rand) * 0.25);
    int o = x & 15;
    int p = z & 15;

    for (int q = 255; q >= 0; --q) {//reduce iteration using ported heightmap for performance??? instead of 255 always
        int lv4 = chunkPrimerIn.getBlock(o, q, p);
        if (lv4 == AIR) {
            m = -1;
        }
        else if (lv4 == STONE) {
            if (m == -1) {
                if (n <= 0) {
                    lv = AIR;
                    lv2 = STONE;
                }
                else if (q >= l - 4 && q <= l + 1) {
                    lv = top_block_;
                    lv2 = filler_block_;
                }

                if (q < l && lv == AIR) {
                    lv = WATER;
                }

                m = n;
                if (q >= l - 1) {
                    chunkPrimerIn.setBlock(o, q, p, lv);
                }
                else if (q < l - 7 - n) {
                    lv = AIR;
                    lv2 = STONE;
                    chunkPrimerIn.setBlock(o, q, p, underwater_block_);
                }
                else {
                    chunkPrimerIn.setBlock(o, q, p, lv2);
                }
            }
            else if (m > 0) {
                --m;
                chunkPrimerIn.setBlock(o, q, p, lv2);
                if (m == 0 && lv2 == SAND && n > 1) {
                    m = nextInt(rand, 4) + std::max(0, q - 63);
                    lv2 = lv2 == RED_SAND ? RED_SANDSTONE : SANDSTONE;
                }
            }
        }
    }
}

void ChunkGenerator::buildBedrock113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
    //in 1.13+, buildBedrock called after surface building
    //1.14+ and 1.13 have different bedrock gen
}

void ChunkGenerator::buildBedrock114(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
}

void ChunkGenerator::replaceBiomeBlocks(int64_t* rand, int x, int z, ChunkData& primer)
{
    double d0 = 0.03125;
    double* depthBuffer = surfaceNoise->getRegion(nullptr, 0, (double)(x * 16), (double)(z * 16), 16, 16, 0.0625, 0.0625, 1.0);

    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            int biome = biomesForGeneration2[j + i * 16];
            int ternary;
            if (std::end(biome_to_ternary_) != biome_to_ternary_.find(biome))
                ternary = biome_to_ternary_[biome];
            else
                throw std::runtime_error("biome_to_ternary__, invalid mapping for " + std::to_string(biome));

            if (std::end(ternary_to_blocktypes_) != ternary_to_blocktypes_.find(ternary))
            {
                auto& ternary_blocks = ternary_to_blocktypes_[ternary];
                top_block_ = std::get<0>(ternary_blocks);
                filler_block_ = std::get<1>(ternary_blocks);
                underwater_block_ = std::get<2>(ternary_blocks);
            }
            else
                throw std::runtime_error("ternary_to_blocktypes_, invalid mapping for " + std::to_string(ternary));

            surface_builder_to_additions_func_[biome_to_surface_builder_[biome]](rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);

            if (std::end(no_def_surface_building_) == no_def_surface_building_.find(biome_to_surface_builder_[biome]))
            {
                if (version_ == MC_1_12)
                {
                    generateBiomeTerrain112(rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);
                }
                else
                {
                    defaultSurfaceBuild113(rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);
                    if(version_ == MC_1_13)
                        buildBedrock113(rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);
                    else
                        buildBedrock114(rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);
                }
            }
        }
    }

    delete[] depthBuffer;
}

void ChunkGenerator::registerBaseAndVariation()
{
    biome_to_base_and_variation_[0] = std::make_pair(-1.0F, 0.1F);
    biome_to_base_and_variation_[1] = std::make_pair(0.125F, 0.05F);
    biome_to_base_and_variation_[2] = std::make_pair(0.125F, 0.05F);
    biome_to_base_and_variation_[3] = std::make_pair(1.0F, 0.5F);
    biome_to_base_and_variation_[4] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[5] = std::make_pair(0.2F, 0.2F);
    biome_to_base_and_variation_[6] = std::make_pair(-0.2F, 0.1F);
    biome_to_base_and_variation_[7] = std::make_pair(-0.5F, 0.0F);
    biome_to_base_and_variation_[8] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[9] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[10] = std::make_pair(-1.0F, 0.1F);
    biome_to_base_and_variation_[11] = std::make_pair(-0.5F, 0.0F);
    biome_to_base_and_variation_[12] = std::make_pair(0.125F, 0.05F);
    biome_to_base_and_variation_[13] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[14] = std::make_pair(0.2F, 0.3F);
    biome_to_base_and_variation_[15] = std::make_pair(0.0F, 0.025F);
    biome_to_base_and_variation_[16] = std::make_pair(0.0F, 0.025F);
    biome_to_base_and_variation_[17] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[18] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[19] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[20] = std::make_pair(0.8F, 0.3F);
    biome_to_base_and_variation_[21] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[22] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[23] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[24] = std::make_pair(-1.8F, 0.1F);
    biome_to_base_and_variation_[25] = std::make_pair(0.1F, 0.8F);
    biome_to_base_and_variation_[26] = std::make_pair(0.0F, 0.025F);
    biome_to_base_and_variation_[27] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[28] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[29] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[30] = std::make_pair(0.2F, 0.2F);
    biome_to_base_and_variation_[31] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[32] = std::make_pair(0.2F, 0.2F);
    biome_to_base_and_variation_[33] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[34] = std::make_pair(1.0F, 0.5F);
    biome_to_base_and_variation_[35] = std::make_pair(0.125F, 0.05F);
    biome_to_base_and_variation_[36] = std::make_pair(1.5F, 0.025F);
    biome_to_base_and_variation_[37] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[38] = std::make_pair(1.5F, 0.025F);
    biome_to_base_and_variation_[39] = std::make_pair(1.5F, 0.025F);

    //post 1.12:
    biome_to_base_and_variation_[40] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[41] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[42] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[43] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[44] = std::make_pair(-1.0F, 0.1F);
    biome_to_base_and_variation_[45] = std::make_pair(-1.0F, 0.1F);
    biome_to_base_and_variation_[46] = std::make_pair(-1.0F, 0.1F);
    biome_to_base_and_variation_[47] = std::make_pair(-1.8F, 0.1F);
    biome_to_base_and_variation_[48] = std::make_pair(-1.8F, 0.1F);
    biome_to_base_and_variation_[49] = std::make_pair(-1.8F, 0.1F);
    biome_to_base_and_variation_[50] = std::make_pair(-1.8F, 0.1F);


    biome_to_base_and_variation_[127] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[129] = std::make_pair(0.125F, 0.05F);
    biome_to_base_and_variation_[130] = std::make_pair(0.225F, 0.25F);
    biome_to_base_and_variation_[131] = std::make_pair(1.0F, 0.5F);
    biome_to_base_and_variation_[132] = std::make_pair(0.1F, 0.4F);
    biome_to_base_and_variation_[133] = std::make_pair(0.3F, 0.4F);
    biome_to_base_and_variation_[134] = std::make_pair(-0.1F, 0.3F);
    biome_to_base_and_variation_[140] = std::make_pair(0.425F, 0.45000002F);
    biome_to_base_and_variation_[149] = std::make_pair(0.2F, 0.4F);
    biome_to_base_and_variation_[151] = std::make_pair(0.2F, 0.4F);
    biome_to_base_and_variation_[155] = std::make_pair(0.2F, 0.4F);
    biome_to_base_and_variation_[156] = std::make_pair(0.55F, 0.5F);
    biome_to_base_and_variation_[157] = std::make_pair(0.2F, 0.4F);
    biome_to_base_and_variation_[158] = std::make_pair(0.3F, 0.4F);
    biome_to_base_and_variation_[160] = std::make_pair(0.2F, 0.2F);
    biome_to_base_and_variation_[161] = std::make_pair(0.2F, 0.2F);
    biome_to_base_and_variation_[162] = std::make_pair(1.0F, 0.5F);
    biome_to_base_and_variation_[163] = std::make_pair(0.3625F, 1.225F);
    biome_to_base_and_variation_[164] = std::make_pair(1.05F, 1.2125001F);
    biome_to_base_and_variation_[165] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[166] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[167] = std::make_pair(0.45F, 0.3F);

    //post 1.12
    biome_to_base_and_variation_[168] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[169] = std::make_pair(0.45F, 0.3F);
    biome_to_base_and_variation_[170] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[171] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[172] = std::make_pair(0.1F, 0.2F);
    biome_to_base_and_variation_[173] = std::make_pair(0.1F, 0.2F);

}

float ChunkGenerator::getBaseHeight(int biome)
{
    if (std::end(biome_to_base_and_variation_) == biome_to_base_and_variation_.find(biome))
        throw std::runtime_error("biome_to_base_and_variation_, no mapping for biome id " + std::to_string(biome));
    return biome_to_base_and_variation_[biome].first;
}
float ChunkGenerator::getHeightVariation(int biome)
{
    if (std::end(biome_to_base_and_variation_) == biome_to_base_and_variation_.find(biome))
        throw std::runtime_error("biome_to_base_and_variation_, no mapping for biome id " + std::to_string(biome));
    return biome_to_base_and_variation_[biome].second;
}

void ChunkGenerator::registerTernaryIds()
{
    biome_to_ternary_[0] = GRASS_T;
    biome_to_ternary_[1] = GRASS_T;
    biome_to_ternary_[2] = SAND_T;
    biome_to_ternary_[3] = GRASS_T;
    biome_to_ternary_[4] = GRASS_T;
    biome_to_ternary_[5] = GRASS_T;
    biome_to_ternary_[6] = GRASS_T;
    biome_to_ternary_[7] = GRASS_T;
    biome_to_ternary_[8] = NETHER_T;
    biome_to_ternary_[9] = END_T;
    biome_to_ternary_[10] = GRASS_T;
    biome_to_ternary_[11] = GRASS_T;
    biome_to_ternary_[12] = GRASS_T;
    biome_to_ternary_[13] = GRASS_T;
    biome_to_ternary_[14] = MYCELIUM_T;
    biome_to_ternary_[15] = MYCELIUM_T;
    biome_to_ternary_[16] = SAND_T;
    biome_to_ternary_[17] = SAND_T;
    biome_to_ternary_[18] = GRASS_T;
    biome_to_ternary_[19] = GRASS_T;
    biome_to_ternary_[20] = GRASS_T;
    biome_to_ternary_[21] = GRASS_T;
    biome_to_ternary_[22] = GRASS_T;
    biome_to_ternary_[23] = GRASS_T;
    biome_to_ternary_[24] = GRASS_T;
    biome_to_ternary_[25] = STONE_T;
    biome_to_ternary_[26] = SAND_T;
    biome_to_ternary_[27] = GRASS_T;
    biome_to_ternary_[28] = GRASS_T;
    biome_to_ternary_[29] = GRASS_T;
    biome_to_ternary_[30] = GRASS_T;
    biome_to_ternary_[31] = GRASS_T;
    biome_to_ternary_[32] = GRASS_T;
    biome_to_ternary_[33] = GRASS_T;
    biome_to_ternary_[34] = GRASS_T;
    biome_to_ternary_[35] = GRASS_T;
    biome_to_ternary_[36] = GRASS_T;
    biome_to_ternary_[37] = MESA_T;
    biome_to_ternary_[38] = MESA_T;
    biome_to_ternary_[39] = MESA_T;

    //post 1.12:
    biome_to_ternary_[40] = END_T;
    biome_to_ternary_[41] = END_T;
    biome_to_ternary_[42] = END_T;
    biome_to_ternary_[43] = END_T;
    biome_to_ternary_[44] = SAND_SAND_UNDERWATER_T;
    biome_to_ternary_[45] = GRASS_SAND_UNDERWATER_T;
    biome_to_ternary_[46] = GRASS_T;
    biome_to_ternary_[47] = SAND_SAND_UNDERWATER_T;
    biome_to_ternary_[48] = GRASS_SAND_UNDERWATER_T;
    biome_to_ternary_[49] = GRASS_T;
    biome_to_ternary_[50] = GRASS_T;


    biome_to_ternary_[127] = STONE_T;
    biome_to_ternary_[129] = GRASS_T;
    biome_to_ternary_[130] = SAND_T;
    biome_to_ternary_[131] = GRASS_T;
    biome_to_ternary_[132] = GRASS_T;
    biome_to_ternary_[133] = GRASS_T;
    biome_to_ternary_[134] = GRASS_T;
    biome_to_ternary_[140] = ICE_SPIKES_T;
    biome_to_ternary_[149] = GRASS_T;
    biome_to_ternary_[151] = GRASS_T;
    biome_to_ternary_[155] = GRASS_T;
    biome_to_ternary_[156] = GRASS_T;
    biome_to_ternary_[157] = GRASS_T;
    biome_to_ternary_[158] = GRASS_T;
    biome_to_ternary_[160] = GRASS_T;
    biome_to_ternary_[161] = GRASS_T;
    biome_to_ternary_[162] = GRASS_T;
    biome_to_ternary_[163] = GRASS_T;
    biome_to_ternary_[164] = GRASS_T;
    biome_to_ternary_[165] = MESA_T;
    biome_to_ternary_[166] = MESA_T;
    biome_to_ternary_[167] = MESA_T;

    //post 1.12
    biome_to_ternary_[168] = GRASS_T;
    biome_to_ternary_[169] = GRASS_T;
    biome_to_ternary_[170] = SOUL_SAND_T;
    biome_to_ternary_[171] = CRIMSON_NYLIUM_T;
    biome_to_ternary_[172] = WARPED_NYLIUM_T;
    //biome_to_ternary_[173], this is basalt deltas
}

void ChunkGenerator::registerTernaryBlocks()
{
    ternary_to_blocktypes_[AIR_T] = std::make_tuple(AIR, AIR, AIR);
    ternary_to_blocktypes_[PODZOL_T] = std::make_tuple(PODZOL, DIRT, GRAVEL);
    ternary_to_blocktypes_[GRAVEL_T] = std::make_tuple(GRAVEL, GRAVEL, GRAVEL);
    ternary_to_blocktypes_[GRASS_T] = std::make_tuple(GRASS, DIRT, GRAVEL);
    ternary_to_blocktypes_[DIRT_T] = std::make_tuple(DIRT, DIRT, GRAVEL);
    ternary_to_blocktypes_[STONE_T] = std::make_tuple(STONE, STONE, GRAVEL);
    ternary_to_blocktypes_[COARSE_DIRT_T] = std::make_tuple(COARSE_DIRT, DIRT, GRAVEL);
    ternary_to_blocktypes_[SAND_T] = std::make_tuple(SAND, SAND, GRAVEL);
    ternary_to_blocktypes_[GRASS_SAND_UNDERWATER_T] = std::make_tuple(GRASS, DIRT, SAND);
    ternary_to_blocktypes_[SAND_SAND_UNDERWATER_T] = std::make_tuple(SAND, SAND, SAND);
    ternary_to_blocktypes_[MESA_T] = std::make_tuple(RED_SAND, HARDCLAY, GRAVEL);
    ternary_to_blocktypes_[MYCELIUM_T] = std::make_tuple(MYCELIUM, DIRT, GRAVEL);
    //ternary_to_blocktypes_[NETHER_T] = std::make_tuple(NETHERRACK, NETHERRACK, NETHERRACK); //dimension support later
    //ternary_to_blocktypes_[SOUL_SAND_T] = std::make_tuple(SOUL_SAND, SOUL_SAND, SOUL_SAND);
    //ternary_to_blocktypes_[END_T] = std::make_tuple(END_STONE, END_STONE, END_STONE);
    //ternary_to_blocktypes_[CRIMSON_NYLIUM_T] = std::make_tuple(AIR, AIR, AIR);
    //ternary_to_blocktypes_[WARPED_NYLIUM_T] = std::make_tuple(AIR, AIR, AIR);
    ternary_to_blocktypes_[ICE_SPIKES_T] = std::make_tuple(SNOW_BLOCK, DIRT, GRAVEL);
}

void ChunkGenerator::registerSurfaceBuilderIds()//don't register default
{
    biome_to_surface_builder_[3] = MOUNTAIN_S;
    biome_to_surface_builder_[163] = SHATTERED_SAVANNA_S;
    biome_to_surface_builder_[164] = SHATTERED_SAVANNA_S;
    biome_to_surface_builder_[131] = GRAVELLY_MOUNTAIN_S;
    biome_to_surface_builder_[162] = GRAVELLY_MOUNTAIN_S;
    biome_to_surface_builder_[160] = GIANT_TREE_TAIGA_S;
    biome_to_surface_builder_[161] = GIANT_TREE_TAIGA_S;
    biome_to_surface_builder_[32] = GIANT_TREE_TAIGA_S;
    biome_to_surface_builder_[33] = GIANT_TREE_TAIGA_S;
    biome_to_surface_builder_[6] = SWAMP_S;
    biome_to_surface_builder_[134] = SWAMP_S;
    biome_to_surface_builder_[37] = MESA_S;
    biome_to_surface_builder_[39] = MESA_S;
    biome_to_surface_builder_[167] = MESA_S;
    biome_to_surface_builder_[38] = WOODED_MESA_S;
    biome_to_surface_builder_[166] = WOODED_MESA_S;
    biome_to_surface_builder_[165] = ERODED_MESA_S;
    biome_to_surface_builder_[10] = FROZEN_OCEAN_S;
    biome_to_surface_builder_[50] = FROZEN_OCEAN_S;
    //type 20/34 are mountain/hills, but uses default
    //type 3/19/30/31 are taiga but use default
}

void ChunkGenerator::registerSurfaceBuilderFuncs(MCversion version)
{
    surface_builder_to_additions_func_[MOUNTAIN_S] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->HillsMountainsSB(a, b, c, d, e, false); };
    surface_builder_to_additions_func_[SHATTERED_SAVANNA_S] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->SavannaMutatedSB(a, b, c, d, e); };
    surface_builder_to_additions_func_[GRAVELLY_MOUNTAIN_S] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->HillsMountainsSB(a, b, c, d, e, true); };
    surface_builder_to_additions_func_[GIANT_TREE_TAIGA_S] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->TaigaSB(a, b, c, d, e); };
    surface_builder_to_additions_func_[SWAMP_S] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->SwampSB(a, b, c, d, e); };

    //no running default surface building after mesa/badlands, still need bedrock gen though
    if (version == MC_1_12)
    {
        surface_builder_to_additions_func_[MESA_S] = [this, version](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaSB112(a, b, c, d, e, false, false); };
        surface_builder_to_additions_func_[WOODED_MESA_S] = [this, version](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaSB112(a, b, c, d, e, false, true); };
        surface_builder_to_additions_func_[ERODED_MESA_S] = [this, version](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaSB112(a, b, c, d, e, true, false); };
    }
    else
    {
        surface_builder_to_additions_func_[MESA_S] = [this, version](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaDefSB113(a, b, c, d, e); };
        surface_builder_to_additions_func_[WOODED_MESA_S] = [this, version](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaWoodedSB113(a, b, c, d, e); };
        surface_builder_to_additions_func_[ERODED_MESA_S] = [this, version](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaErodedSB113(a, b, c, d, e); };
    }

    surface_builder_to_additions_func_[FROZEN_OCEAN_S] = [this](int64_t* , ChunkData& , int , int , double ) { };//frozen ocean has temp dependencies, don't support right now
}

void ChunkGenerator::HillsMountainsSB(int64_t*, ChunkData&, int, int, double noiseVal, bool mutated) //hills were changed to mountains in 1.13. biome ids 3, 20, 34, 131, 162
{
    top_block_ = GRASS;
    filler_block_ = DIRT;
    if ((noiseVal < -1.0 || noiseVal > 2.0) && mutated) {
        top_block_ = GRAVEL;
        filler_block_ = GRAVEL;
    }
    else if (noiseVal > 1.0) {
        top_block_ = STONE;
        filler_block_ = STONE;
    }
}

void ChunkGenerator::MesaSB112(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal, bool bryce/*'eroded 1.13+*/, bool forest/*'wooded' 1.13+*/)
{
    double d4 = 0.0;
    int k1;
    int l1;
    if (bryce) {
        k1 = (x & -16) + (z & 15);
        l1 = (z & -16) + (x & 15);
        double d0 = std::min(std::abs(noiseVal), mesaPillarNoise->getValue((double)k1 * 0.25, (double)l1 * 0.25));
        if (d0 > 0.0) {
            double d1 = 0.001953125;
            double d2 = std::abs(mesaPillarRoofNoise->getValue((double)k1 * 0.001953125, (double)l1 * 0.001953125));
            d4 = d0 * d0 * 2.5;
            double d3 = std::ceil(d2 * 50.0) + 14.0;
            if (d4 > d3) {
                d4 = d3;
            }

            d4 += 64.0;
        }
    }

    k1 = x & 15;
    l1 = z & 15;
    int i2 = 63;
    int iblockstate = HARDCLAY;
    int iblockstate3 = filler_block_;
    int k = (int)(noiseVal / 3.0 + 3.0 + nextDouble(rand) * 0.25);
    bool flag = cos(noiseVal / 3.0 * 3.141592653589793) > 0.0;
    int l = -1;
    bool flag1 = false;
    int i1 = 0;

    for (int j1 = 255; j1 >= 0; --j1) {
        if (chunkPrimerIn.getBlock(l1, j1, k1) == AIR && j1 < (int)d4) {
            chunkPrimerIn.setBlock(l1, j1, k1, STONE);
        }

        if (j1 <= nextInt(rand, 5)) {
            chunkPrimerIn.setBlock(l1, j1, k1, BEDROCK);
        }
        else if (i1 < 15 || bryce) {
            int iblockstate1 = chunkPrimerIn.getBlock(l1, j1, k1);
            if (iblockstate1 == AIR) {
                l = -1;
            }
            else if (iblockstate1 == STONE) {
                if (l == -1) {
                    flag1 = false;
                    if (k <= 0) {
                        iblockstate = AIR;
                        iblockstate3 = STONE;
                    }
                    else if (j1 >= i2 - 4 && j1 <= i2 + 1) {
                        iblockstate = HARDCLAY;
                        iblockstate3 = filler_block_;
                    }

                    if (j1 < i2 && (iblockstate == AIR)) {
                        iblockstate = WATER;
                    }

                    l = k + std::max(0, j1 - i2);
                    if (j1 >= i2 - 1) {
                        if (forest && j1 > 86 + k * 2) {
                            if (flag) {
                                chunkPrimerIn.setBlock(l1, j1, k1, COARSE_DIRT);
                            }
                            else {
                                chunkPrimerIn.setBlock(l1, j1, k1, GRASS);
                            }
                        }
                        else if (j1 > i2 + 3 + k) {
                            int iblockstate2;
                            if (j1 >= 64 && j1 <= 127) {
                                if (flag) {
                                    iblockstate2 = HARDCLAY;
                                }
                                else {
                                    iblockstate2 = HARDCLAY;
                                }
                            }
                            else {
                                iblockstate2 = HARDCLAY;
                            }

                            chunkPrimerIn.setBlock(l1, j1, k1, iblockstate2);
                        }
                        else {
                            chunkPrimerIn.setBlock(l1, j1, k1, top_block_);
                            flag1 = true;
                        }
                    }
                    else {
                        chunkPrimerIn.setBlock(l1, j1, k1, iblockstate3);
                        if (iblockstate3 == HARDCLAY) {
                            chunkPrimerIn.setBlock(l1, j1, k1, HARDCLAY);
                        }
                    }
                }
                else if (l > 0) {
                    --l;
                    if (flag1) {
                        chunkPrimerIn.setBlock(l1, j1, k1, HARDCLAY);
                    }
                    else {
                        chunkPrimerIn.setBlock(l1, j1, k1, HARDCLAY);
                    }
                }

                ++i1;
            }
        }
    }
}

void ChunkGenerator::MesaDefSB113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
    int n = x & 15;
    int o = z & 15;
    int l = 63;
    int lv = HARDCLAY;
    int lv2 = filler_block_;
    int p = (int)(noiseVal / 3.0 + 3.0 + nextDouble(rand) * 0.25);
    bool bl = cos(noiseVal / 3.0 * 3.141592653589793) > 0.0;
    int q = -1;
    bool bl2 = false;
    int r = 0;

    for (int s = 255; s >= 0; --s) {
        if (r < 15) {
            //lv3 is n, s, o coord
            int lv4 = chunkPrimerIn.getBlock(n, s, o);
            if (lv4 == AIR) {
                q = -1;
            }
            else if (lv4 == STONE) {
                if (q == -1) {
                    bl2 = false;
                    if (p <= 0) {
                        lv = AIR;
                        lv2 = STONE;
                    }
                    else if (s >= l - 4 && s <= l + 1) {
                        lv = HARDCLAY;
                        lv2 = filler_block_;
                    }

                    if (s < l && (lv == AIR)) {
                        lv = WATER;
                    }

                    q = p + std::max(0, s - l);
                    if (s >= l - 1) {
                        if (s > l + 3 + p) {
                            int lv7;
                            if (s >= 64 && s <= 127) {
                                if (bl) {
                                    lv7 = HARDCLAY;
                                }
                                else {
                                    lv7 = HARDCLAY;
                                }
                            }
                            else {
                                lv7 = HARDCLAY;
                            }

                            chunkPrimerIn.setBlock(n, s, o, lv7);
                        }
                        else {
                            chunkPrimerIn.setBlock(n, s, o, top_block_);
                            bl2 = true;
                        }
                    }
                    else {
                        chunkPrimerIn.setBlock(n, s, o, lv2);
                        int lv8 = lv2;
                        if (lv8 == HARDCLAY) {
                            chunkPrimerIn.setBlock(n, s, o, HARDCLAY);
                        }
                    }
                }
                else if (q > 0) {
                    --q;
                    if (bl2) {
                        chunkPrimerIn.setBlock(n, s, o, HARDCLAY);
                    }
                    else {
                        chunkPrimerIn.setBlock(n, s, o, HARDCLAY);
                    }
                }

                ++r;
            }
        }
    }
}
void ChunkGenerator::MesaWoodedSB113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
    int n = x & 15;
    int o = z & 15;
    int l = 63;
    int lv = HARDCLAY;
    int lv2 = filler_block_;
    int p = (int)(noiseVal / 3.0 + 3.0 + nextDouble(rand) * 0.25);
    bool bl = cos(noiseVal / 3.0 * 3.141592653589793) > 0.0;
    int q = -1;
    bool bl2 = false;
    int r = 0;

    for (int s = 255; s >= 0; --s) {
        if (r < 15) {
            int lv4 = chunkPrimerIn.getBlock(n, s, o);
            if (lv4 == AIR) {
                q = -1;
            }
            else if (lv4 == STONE) {
                if (q == -1) {
                    bl2 = false;
                    if (p <= 0) {
                        lv = AIR;
                        lv2 = STONE;
                    }
                    else if (s >= l - 4 && s <= l + 1) {
                        lv = HARDCLAY;
                        lv2 = filler_block_;
                    }

                    if (s < l && lv == AIR) {
                        lv = WATER;
                    }

                    q = p + std::max(0, s - l);
                    if (s >= l - 1) {
                        if (s > 86 + p * 2) {
                            if (bl) {
                                chunkPrimerIn.setBlock(n, s, o, COARSE_DIRT);
                            }
                            else {
                                chunkPrimerIn.setBlock(n, s, o, GRASS);
                            }
                        }
                        else if (s > l + 3 + p) {
                            int lv7;
                            if (s >= 64 && s <= 127) {
                                if (bl) {
                                    lv7 = HARDCLAY;
                                }
                                else {
                                    lv7 = HARDCLAY;
                                }
                            }
                            else {
                                lv7 = HARDCLAY;
                            }

                            chunkPrimerIn.setBlock(n, s, o, lv7);
                        }
                        else {
                            chunkPrimerIn.setBlock(n, s, o, top_block_);
                            bl2 = true;
                        }
                    }
                    else {
                        chunkPrimerIn.setBlock(n, s, o, lv2);
                        if (lv2 == HARDCLAY) {
                            chunkPrimerIn.setBlock(n, s, o, HARDCLAY);
                        }
                    }
                }
                else if (q > 0) {
                    --q;
                    if (bl2) {
                        chunkPrimerIn.setBlock(n, s, o, HARDCLAY);
                    }
                    else {
                        chunkPrimerIn.setBlock(n, s, o, HARDCLAY);
                    }
                }

                ++r;
            }
        }
    }
}
void ChunkGenerator::MesaErodedSB113(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
{
    double e = 0.0;
    double f = std::min(std::abs(noiseVal), mesaPillarNoise->getValue((double)x * 0.25, (double)z * 0.25) * 15.0);
    if (f > 0.0) {
        double g = 0.001953125;
        double h = std::abs(mesaPillarRoofNoise->getValue((double)x * 0.001953125, (double)z * 0.001953125));
        e = f * f * 2.5;
        double n = std::ceil(h * 50.0) + 14.0;
        if (e > n) {
            e = n;
        }

        e += 64.0;
    }

    int o = x & 15;
    int p = z & 15;
    int l = 63;
    int lv = HARDCLAY;
    int lv2 = filler_block_;
    int q = (int)(noiseVal / 3.0 + 3.0 + nextDouble(rand) * 0.25);
    bool bl = cos(noiseVal / 3.0 * 3.141592653589793) > 0.0;
    int r = -1;
    bool bl2 = false;

    for (int s = 255; s >= 0; --s) {
        if (chunkPrimerIn.getBlock(o, s, p) == AIR && s < (int)e) {
            chunkPrimerIn.setBlock(o, s, p, STONE);
        }

        int lv4 = chunkPrimerIn.getBlock(o, s, p);
        if (lv4 == AIR) {
            r = -1;
        }
        else if (lv4 == STONE) {
            if (r == -1) {
                bl2 = false;
                if (q <= 0) {
                    lv = AIR;
                    lv2 = STONE;
                }
                else if (s >= l - 4 && s <= l + 1) {
                    lv = HARDCLAY;
                    lv2 = filler_block_;
                }

                if (s < l && lv == AIR) {
                    lv = WATER;
                }

                r = q + std::max(0, s - l);
                if (s >= l - 1) {
                    if (s <= l + 3 + q) {
                        chunkPrimerIn.setBlock(o, s, p, top_block_);
                        bl2 = true;
                    }
                    else {
                        int lv7;
                        if (s >= 64 && s <= 127) {
                            if (bl) {
                                lv7 = HARDCLAY;
                            }
                            else {
                                lv7 = HARDCLAY;
                            }
                        }
                        else {
                            lv7 = HARDCLAY;
                        }

                        chunkPrimerIn.setBlock(o, s, p, lv7);
                    }
                }
                else {
                    chunkPrimerIn.setBlock(o, s, p, lv2);
                    int lv8 = lv2;
                    if (lv8 == HARDCLAY) {
                        chunkPrimerIn.setBlock(o, s, p, HARDCLAY);
                    }
                }
            }
            else if (r > 0) {
                --r;
                if (bl2) {
                    chunkPrimerIn.setBlock(o, s, p, HARDCLAY);
                }
                else {
                    chunkPrimerIn.setBlock(o, s, p, HARDCLAY);
                }
            }
        }
    }
}

void ChunkGenerator::SwampSB(int64_t*, ChunkData& chunkPrimerIn, int x, int z, double)
{
    double d0 = grassColorNoise->getValue((double)x * 0.25, (double)z * 0.25);
    if (d0 > 0.0) {
        int i = x & 15;
        int j = z & 15;

        for (int k = 255; k >= 0; --k) {
            if (chunkPrimerIn.getBlock(j, k, i) != AIR) {
                if (k == 62 && chunkPrimerIn.getBlock(j, k, i) != WATER) {
                    chunkPrimerIn.setBlock(j, k, i, WATER);
                    if (d0 < 0.12) {
                        chunkPrimerIn.setBlock(j, k + 1, i, WATER_LILY);
                    }
                }
                break;
            }
        }
    }
}
void ChunkGenerator::TaigaSB(int64_t*, ChunkData&, int, int, double noiseVal)
{
    top_block_ = GRASS;
    filler_block_ = DIRT;
    if (noiseVal > 1.75) {
        top_block_ = COARSE_DIRT;
    }
    else if (noiseVal > -0.95) {
        top_block_ = PODZOL;
    }
}
void ChunkGenerator::SavannaMutatedSB(int64_t*, ChunkData&, int, int, double noiseVal)
{
    top_block_ = GRASS;
    filler_block_ = DIRT;
    if (noiseVal > 1.75) {
        top_block_ = STONE;
        filler_block_ = STONE;
    }
    else if (noiseVal > -0.5) {
        top_block_ = COARSE_DIRT;
    }
}