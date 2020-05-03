#include "ChunkGenerator.h"
#include <cmath>

ChunkGenerator::ChunkGenerator(int64_t world_seed, int64_t* randprimedseed):
    minLimitPerlinNoise(randprimedseed, 16),
    maxLimitPerlinNoise(randprimedseed, 16),
    mainPerlinNoise(randprimedseed, 8),
    surfaceNoise(randprimedseed, 4),
    scaleNoise(randprimedseed, 10),
    depthNoise(randprimedseed, 16),
    forestNoise(randprimedseed, 8)
{
    for (int i = -2; i <= 2; ++i)
    {
        for (int j = -2; j <= 2; ++j)
        {
            float f = 10.0F / sqrt((float)(i * i + j * j) + 0.2F);
            biomeWeights[i + 2 + (j + 2) * 5] = f;
        }
    }
    biome_base_height_ = 0.125F; //plains
    biome_height_variation_ = 0.05F; //plains
    amplified_ = false;

    top_block_ = GRASS;
    filler_block_ = DIRT;

    int64_t tempseed = world_seed;
    setSeed(&tempseed);
    mesaPillarNoise = std::make_unique<NoiseGeneratorPerlin>(&tempseed, 4);
    mesaPillarRoofNoise = std::make_unique<NoiseGeneratorPerlin>(&tempseed, 1);

    int64_t grassNoiseSeed = 2345L;
    setSeed(&grassNoiseSeed);
    grassColorNoise = std::make_unique<NoiseGeneratorPerlin>(&grassNoiseSeed, 1);

    initBiomes();

    stack_ = setupGenerator(MC_1_12);
    applySeed(&stack_, world_seed);

    registerBaseAndVariation();
    registerBiomeIdTypeMappings();
    registerTopAndFiller();
    registerGenTerrainFuncs();

    dont_generate_biome_terrain.insert(MESANORMAL);
    dont_generate_biome_terrain.insert(MESABRYCE);
    dont_generate_biome_terrain.insert(MESAFOREST);
    dont_generate_biome_terrain.insert(MESABRYCEFOREST);
}

void ChunkGenerator::provideChunk(int x, int z, ChunkData& chunk, std::unordered_set<int>* biomes)
{
    
    int* map = allocCache(&stack_.layers[L_VORONOI_ZOOM_1], 1, 1);
    genArea(&stack_.layers[L_VORONOI_ZOOM_1], map, x*16, z*16, 1, 1);

    if (biomes)
    {
        if (std::end(*biomes) == biomes->find(*map))
            return;
    }

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            for (int k = 0; k < 256; k++)
                chunk.setBlock(i, k, j, ChunkGenerator::AIR);

    if (std::end(biome_to_base_and_variation_) == biome_to_base_and_variation_.find(*map))
        throw std::runtime_error("Mapping Doesn't Exist #0");
    biome_base_height_ = biome_to_base_and_variation_[*map].first;
    biome_height_variation_ = biome_to_base_and_variation_[*map].second;
    
    if (std::end(biome_id_to_biome_type_) == biome_id_to_biome_type_.find(*map))
        throw std::runtime_error("Mapping Doesn't Exist #1");
    int biome_type = biome_id_to_biome_type_[*map];

    if (std::end(biome_to_top_and_filler_) == biome_to_top_and_filler_.find(biome_type))
        throw std::runtime_error("Mapping Doesn't Exist #2");
    top_block_ = biome_to_top_and_filler_[biome_type].first;
    filler_block_ = biome_to_top_and_filler_[biome_type].second;

    int64_t rand = (int64_t)x * 341873128712L + (int64_t)z * 132897987541L;
    setSeed(&rand);
    setBlocksInChunk(x, z, chunk);
    replaceBiomeBlocks(&rand, x, z, chunk, *map);

    free(map);
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
                            if (i * 4 + k2 == 2 && l * 4 + l2 == 0 && i2 * 8 + j2 == 6)
                                i = i;
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
    double* depthRegion = depthNoise.generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_3_, 5, 5, depthNoiseScaleX, depthNoiseScaleZ, depthNoiseScaleExponent);
    float f = coordinateScale;
    float f1 = heightScale;
    double* mainNoiseRegion = mainPerlinNoise.generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_2_, p_185978_3_, 5, 33, 5, (double)(f / mainNoiseScaleX), (double)(f1 / mainNoiseScaleY), (double)(f / mainNoiseScaleZ));
    double* minLimitRegion = minLimitPerlinNoise.generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_2_, p_185978_3_, 5, 33, 5, (double)f, (double)f1, (double)f);
    double* maxLimitRegion = maxLimitPerlinNoise.generateNoiseOctaves(nullptr, 0, p_185978_1_, p_185978_2_, p_185978_3_, 5, 33, 5, (double)f, (double)f1, (double)f);
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

            for (int j1 = -2; j1 <= 2; ++j1)
            {
                for (int k1 = -2; k1 <= 2; ++k1)
                {
                    float f5 = biomeDepthOffset + biome_base_height_ * biomeDepthWeight;
                    float f6 = biomeScaleOffset + biome_height_variation_ * biomeScaleWeight;

                    if (amplified_ && f5 > 0.0F)
                    {
                        f5 = 1.0F + f5 * 2.0F;
                        f6 = 1.0F + f6 * 4.0F;
                    }

                    float f7 = biomeWeights[j1 + 2 + (k1 + 2) * 5] / (f5 + 2.0F);

                    if (biome_base_height_ > biome_base_height_)
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

void ChunkGenerator::generateBiomeTerrain(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal)
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
                        //iblockstate1 = iblockstate1.getValue(BlockSand.VARIANT) == BlockSand.EnumType.RED_SAND ? RED_SANDSTONE : SANDSTONE;
                        iblockstate1 = SANDSTONE;
                    }
                }
            }
        }
    }
}

void ChunkGenerator::replaceBiomeBlocks(int64_t* rand, int x, int z, ChunkData& primer, int biome)
{
    double d0 = 0.03125;
    double* depthBuffer = surfaceNoise.getRegion(nullptr, 0, (double)(x * 16), (double)(z * 16), 16, 16, 0.0625, 0.0625, 1.0);

    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            if (std::end(biome_type_to_gen_terrain_blocks_) != biome_type_to_gen_terrain_blocks_.find(biome))
                biome_type_to_gen_terrain_blocks_[biome](rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);
            else
                throw std::runtime_error("Invalid Mapping #3");
            if(std::end(dont_generate_biome_terrain) == dont_generate_biome_terrain.find(biome))
                generateBiomeTerrain(rand, primer, x * 16 + i, z * 16 + j, depthBuffer[j + i * 16]);
        }
    }
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

}

void ChunkGenerator::registerBiomeIdTypeMappings()
{
    biome_id_to_biome_type_[0] = OCEAN;
    biome_id_to_biome_type_[1] = PLAINS;
    biome_id_to_biome_type_[2] = DESERT;
    biome_id_to_biome_type_[3] = HILLSNORMAL;
    biome_id_to_biome_type_[4] = FOREST;
    biome_id_to_biome_type_[5] = TAIGANORMAL;
    biome_id_to_biome_type_[6] = SWAMP;
    biome_id_to_biome_type_[7] = RIVER;
    biome_id_to_biome_type_[8] = HELL;
    biome_id_to_biome_type_[9] = END;
    biome_id_to_biome_type_[10] = OCEAN;
    biome_id_to_biome_type_[11] = RIVER;
    biome_id_to_biome_type_[12] = SNOW;
    biome_id_to_biome_type_[13] = SNOW;
    biome_id_to_biome_type_[14] = MUSHROOMISLAND;
    biome_id_to_biome_type_[15] = MUSHROOMISLAND;
    biome_id_to_biome_type_[16] = BEACH;
    biome_id_to_biome_type_[17] = DESERT;
    biome_id_to_biome_type_[18] = FOREST;
    biome_id_to_biome_type_[19] = TAIGANORMAL;
    biome_id_to_biome_type_[20] = HILLSEXTRATREES;
    biome_id_to_biome_type_[21] = JUNGLE;
    biome_id_to_biome_type_[22] = JUNGLE;
    biome_id_to_biome_type_[23] = JUNGLE;
    biome_id_to_biome_type_[24] = OCEAN;
    biome_id_to_biome_type_[25] = STONEBEACH;
    biome_id_to_biome_type_[26] = BEACH;
    biome_id_to_biome_type_[27] = FOREST;
    biome_id_to_biome_type_[28] = FOREST;
    biome_id_to_biome_type_[29] = FOREST;
    biome_id_to_biome_type_[30] = TAIGANORMAL;
    biome_id_to_biome_type_[31] = TAIGANORMAL;
    biome_id_to_biome_type_[32] = TAIGAMEGA;
    biome_id_to_biome_type_[33] = TAIGAMEGA;
    biome_id_to_biome_type_[34] = HILLSEXTRATREES;
    biome_id_to_biome_type_[35] = SAVANNA;
    biome_id_to_biome_type_[36] = SAVANNA;
    biome_id_to_biome_type_[37] = MESANORMAL;
    biome_id_to_biome_type_[38] = MESAFOREST;
    biome_id_to_biome_type_[39] = MESANORMAL;

    biome_id_to_biome_type_[127] = VOID;
    biome_id_to_biome_type_[129] = PLAINS;
    biome_id_to_biome_type_[130] = DESERT;
    biome_id_to_biome_type_[131] = HILLSMUTATED;
    biome_id_to_biome_type_[132] = FOREST;
    biome_id_to_biome_type_[133] = TAIGANORMAL;
    biome_id_to_biome_type_[134] = SWAMP;
    biome_id_to_biome_type_[140] = SNOW;
    biome_id_to_biome_type_[149] = JUNGLE;
    biome_id_to_biome_type_[151] = JUNGLE;
    biome_id_to_biome_type_[155] = FORESTMUTATED;
    biome_id_to_biome_type_[156] = FORESTMUTATED;
    biome_id_to_biome_type_[157] = FOREST;
    biome_id_to_biome_type_[158] = TAIGANORMAL;
    biome_id_to_biome_type_[160] = TAIGAMEGA;
    biome_id_to_biome_type_[161] = TAIGAMEGA;
    biome_id_to_biome_type_[162] = HILLSMUTATED;
    biome_id_to_biome_type_[163] = SAVANNAMUTATED;
    biome_id_to_biome_type_[164] = SAVANNAMUTATED;
    biome_id_to_biome_type_[165] = MESABRYCE;
    biome_id_to_biome_type_[166] = MESAFOREST;
    biome_id_to_biome_type_[167] = MESANORMAL;
}

void ChunkGenerator::registerTopAndFiller()
{
    biome_to_top_and_filler_[BEACH] = std::make_pair(SAND, SAND);
    biome_to_top_and_filler_[DESERT] = std::make_pair(SAND, SAND);
    biome_to_top_and_filler_[END] = std::make_pair(DIRT, DIRT);
    biome_to_top_and_filler_[FOREST] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[HELL] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[HILLSNORMAL] = std::make_pair(GRASS, GRASS);//dependent on noise
    biome_to_top_and_filler_[JUNGLE] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[MESANORMAL] = std::make_pair(SAND, HARDCLAY);
    biome_to_top_and_filler_[MUSHROOMISLAND] = std::make_pair(MYCELIUM, DIRT);
    biome_to_top_and_filler_[OCEAN] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[PLAINS] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[RIVER] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[SAVANNA] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[SNOW] = std::make_pair(SNOW_BLOCK, DIRT);
    biome_to_top_and_filler_[STONEBEACH] = std::make_pair(STONE, STONE);
    biome_to_top_and_filler_[SWAMP] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[TAIGANORMAL] = std::make_pair(GRASS, DIRT);//dep. on noise
    biome_to_top_and_filler_[VOID] = std::make_pair(GRASS, DIRT);

    biome_to_top_and_filler_[FORESTMUTATED] = std::make_pair(GRASS, DIRT);
    biome_to_top_and_filler_[SAVANNAMUTATED] = std::make_pair(GRASS, DIRT); //dep. on noise

    biome_to_top_and_filler_[HILLSEXTRATREES] = std::make_pair(GRASS, GRASS);
    biome_to_top_and_filler_[HILLSMUTATED] = std::make_pair(GRASS, GRASS);

    biome_to_top_and_filler_[MESABRYCE] = std::make_pair(SAND, HARDCLAY);
    biome_to_top_and_filler_[MESAFOREST] = std::make_pair(SAND, HARDCLAY);
    biome_to_top_and_filler_[MESABRYCEFOREST] = std::make_pair(SAND, HARDCLAY);

    biome_to_top_and_filler_[TAIGAMEGA] = std::make_pair(GRASS, DIRT);
}

void ChunkGenerator::registerGenTerrainFuncs()
{
    biome_type_to_gen_terrain_blocks_[BEACH] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->BeachGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[DESERT] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->DesertGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[END] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->EndGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[FOREST] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->ForestGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[HELL] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->HellGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[HILLSNORMAL] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->HillsGTB(a, b, c, d, e, false, false); };
    biome_type_to_gen_terrain_blocks_[JUNGLE] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->JungleGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[MESANORMAL] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaGTB(a, b, c, d, e, false, false); };
    biome_type_to_gen_terrain_blocks_[MUSHROOMISLAND] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->MushroomIslandGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[OCEAN] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->OceanGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[PLAINS] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->PlainsGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[RIVER] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->RiverGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[SAVANNA] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->SavannaGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[SNOW] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->SnowGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[STONEBEACH] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->StoneBeachGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[SWAMP] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->SwampGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[TAIGANORMAL] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->TaigaGTB(a, b, c, d, e, false); };
    biome_type_to_gen_terrain_blocks_[VOID] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->VoidGTB(a, b, c, d, e); };

    biome_type_to_gen_terrain_blocks_[FORESTMUTATED] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->ForestMutatedGTB(a, b, c, d, e); };
    biome_type_to_gen_terrain_blocks_[SAVANNAMUTATED] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->SavannaMutatedGTB(a, b, c, d, e); };

    biome_type_to_gen_terrain_blocks_[HILLSEXTRATREES] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->HillsGTB(a, b, c, d, e, true, false); };
    biome_type_to_gen_terrain_blocks_[HILLSMUTATED] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->HillsGTB(a, b, c, d, e, false, true); };

    biome_type_to_gen_terrain_blocks_[MESABRYCE] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaGTB(a, b, c, d, e, true, false); };
    biome_type_to_gen_terrain_blocks_[MESAFOREST] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaGTB(a, b, c, d, e, false, true); };
    biome_type_to_gen_terrain_blocks_[MESABRYCEFOREST] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->MesaGTB(a, b, c, d, e, true, true); };

    biome_type_to_gen_terrain_blocks_[TAIGAMEGA] = [this](int64_t* a, ChunkData& b, int c, int d, double e) {this->TaigaGTB(a, b, c, d, e, true); };
}

void ChunkGenerator::BeachGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::DesertGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::EndGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::ForestGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::HellGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::HillsGTB(int64_t*, ChunkData&, int, int, double noiseVal, bool extra_trees, bool mutated)
{
    top_block_ = GRASS;
    filler_block_ = GRASS;
    if ((noiseVal < -1.0 || noiseVal > 2.0) && mutated) {
        top_block_ = GRAVEL;
        filler_block_ = GRAVEL;
    }
    else if (noiseVal > 1.0 && !extra_trees) {
        top_block_ = STONE;
        filler_block_ = STONE;
    }
}
void ChunkGenerator::JungleGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::MesaGTB(int64_t* rand, ChunkData& chunkPrimerIn, int x, int z, double noiseVal, bool bryce, bool forest)
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
                                chunkPrimerIn.setBlock(l1, j1, k1, DIRT);
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
void ChunkGenerator::MushroomIslandGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::OceanGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::PlainsGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::RiverGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::SavannaGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::SnowGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::StoneBeachGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::SwampGTB(int64_t*, ChunkData& chunkPrimerIn, int x, int z, double)
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
void ChunkGenerator::TaigaGTB(int64_t*, ChunkData&, int, int, double noiseVal, bool mega)
{
    if (mega) {
        top_block_ = GRASS;
        filler_block_ = DIRT;
        if (noiseVal > 1.75) {
            top_block_ = DIRT;
        }
        else if (noiseVal > -0.95) {
            top_block_ = DIRT;
        }
    }
}
void ChunkGenerator::VoidGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::ForestMutatedGTB(int64_t*, ChunkData&, int, int, double) {}
void ChunkGenerator::SavannaMutatedGTB(int64_t*, ChunkData&, int, int, double noiseVal)
{
    top_block_ = GRASS;
    filler_block_ = DIRT;
    if (noiseVal > 1.75) {
        top_block_ = STONE;
        filler_block_ = STONE;
    }
    else if (noiseVal > -0.5) {
        top_block_ = DIRT;
    }
}