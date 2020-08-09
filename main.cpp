#include "generator.h"
#include "layers.h"
#include <iostream>
#include "ChunkGenerator.h"
#include "Search.h"

void found_main(std::pair<int, int> location)
{
	std::cout << "found cb, X: " << std::get<0>(location) << " Z: " << std::get<1>(location) << std::endl;
}

void progress_main(double progress)
{
	std::cout << "Progress: " << progress << std::endl;
}

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

	/*std::cout<< std::numeric_limits<double>::is_iec559 << std::endl;

	ChunkGenerator generator(8675309, MC_1_12);
	ChunkData chunk;
	ChunkHeightmap chunkheight;
	generator.provideChunk(-11, -3, chunk);*/
	//generator.provideChunkHeightmap(-11, -3, chunkheight, 1/*plains*/);

	/*std::cout << std::endl;

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

	for (int i = 15; i >= 0; i--)
	{
		for (int j = 0; j < 16; j++)
		{
			std::cout << chunkheight.getHeightmap(i, j) << " ";
		}
		std::cout << std::endl;;
	}*/

	std::vector<FormationBlock> formation;
	/*formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(2, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(3, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(4, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 1, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 2, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 4, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(1, 0, 2, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(1, 0, 4, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::WATER_LILY));*/
	//formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::GRASS)); 
	
	formation.push_back(FormationBlock(1, 5, 0, 1));
	formation.push_back(FormationBlock(0, 5, 1, 1));
	formation.push_back(FormationBlock(1, 10, 1, 1));
	formation.push_back(FormationBlock(0, 1, 0, 0));
	formation.push_back(FormationBlock(1, 6, 0, 0));
	formation.push_back(FormationBlock(0, 6, 1, 0));
	formation.push_back(FormationBlock(1, 11, 1, 0));
	SearchFunc func = cachedHeightmapSearch;
	ProgressCallback progresscb = progress_main;
	std::unordered_set<int> biomes;
	//biomes.insert(1);
	//biomes.insert(3);
	//biomes.insert(131);
	//biomes.insert(164);
	//biomes.insert(1);
	//biomes.insert(3);
	threadedSearch(func, 4, -4172144997902289642, MC_1_12, -200, 200, 60, 90, -200, 200, formation, biomes, true, nullptr, &progresscb, found_main);
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