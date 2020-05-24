#include "generator.h"
#include "layers.h"
#include <iostream>
#include "ChunkGenerator.h"
#include "Search.h"

int main()
{
	initBiomes();
	/*LayerStack stack;

	stack = setupGenerator(MC_1_15);
	applySeed(&stack, 8675309LL);
	int* map = allocCache(&stack.layers[L_VORONOI_ZOOM_1], 16, 16);
	genArea(&stack.layers[L_VORONOI_ZOOM_1], map, 9968, 9936, 16, 16);

	for (int i = 15; i >= 0; i--)
	{
		for (int j = 0; j < 16; j++)
		{
			std::cout << map[j*16+i] << " ";
		}
		std::cout << std::endl;
	}*/

	//std::cout << "Biome: " << *map << std::endl;

	std::cout<< std::numeric_limits<double>::is_iec559 << std::endl;

	ChunkGenerator generator(8675309LL, MC_1_13);
	ChunkData chunk;
	generator.provideChunk(26, 37, chunk);

	for (int i = 15; i >= 0; i--)
	{
		for (int j = 0; j < 16; j++)
		{
			int h = 255;
			while (chunk.getBlock(i, h, j) == ChunkGenerator::AIR)h--;

			std::cout << h << " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

	for (int i = 15; i >= 0; i--)
	{
		for (int j = 0; j < 16; j++)
		{
			int h = 255;
			while (chunk.getBlock(i, h, j) == ChunkGenerator::AIR)h--;

			std::cout << chunk.getBlock(i, h, j) << " ";
		}
		std::cout << std::endl;
	}

	/*std::vector<FormationBlock> formation;
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::GRASS));
	formation.push_back(FormationBlock(1, 5, 0, ChunkGenerator::GRASS)); 
	formation.push_back(FormationBlock(0, 5, 1, ChunkGenerator::GRASS));
	formation.push_back(FormationBlock(1, 10, 1, ChunkGenerator::GRASS));
	formation.push_back(FormationBlock(0, 1, 0, ChunkGenerator::AIR));
	formation.push_back(FormationBlock(1, 6, 0, ChunkGenerator::AIR));
	formation.push_back(FormationBlock(0, 6, 1, ChunkGenerator::AIR));
	formation.push_back(FormationBlock(1, 11, 1, ChunkGenerator::AIR));
	TerrainSearchFunc func = cachedSearch;
	threadedSearch(func, 2, 8675309L, -100, 100, 60, 90, -100, 100, formation, std::unordered_set<int>(), true);*/
	//cachedSearch(8675309L, -100, 100, 60, 90, -100, 100, formation, nullptr, true);

	return 0;
}

/*
TODO:
Extra biomes for new versions
--different between all 1.13, 1.14, 1.15??
--bedrock gen probably wrong for existing biomes

Map biome id to surface builder type and block config type separately

hills GTB == mountain SB ???

hills -> mountains


lib generates stone but actual grass, and correct for 1.12
-noiseVal gen bad?
-swamp SB call screwed up?
*/