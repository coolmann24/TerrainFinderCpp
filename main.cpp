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

	/*std::cout<< std::numeric_limits<double>::is_iec559 << std::endl;

	ChunkGenerator generator(8675309LL);
	ChunkData chunk;
	generator.provideChunk(545, 88, chunk);

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
	}*/

	std::vector<FormationBlock> formation;
	formation.push_back(FormationBlock(0, 0, 0, ChunkGenerator::GRASS));
	formation.push_back(FormationBlock(1, 5, 0, ChunkGenerator::GRASS)); 
	formation.push_back(FormationBlock(0, 5, 1, ChunkGenerator::GRASS));
	formation.push_back(FormationBlock(1, 10, 1, ChunkGenerator::GRASS));
	formation.push_back(FormationBlock(0, 1, 0, ChunkGenerator::AIR));
	formation.push_back(FormationBlock(1, 6, 0, ChunkGenerator::AIR));
	formation.push_back(FormationBlock(0, 6, 1, ChunkGenerator::AIR));
	formation.push_back(FormationBlock(1, 11, 1, ChunkGenerator::AIR));
	search(8675309, -100, 100, 60, 90, -100, 100, formation, nullptr, true);

	return 0;
}