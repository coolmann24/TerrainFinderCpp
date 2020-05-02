#include "generator.h"
#include "layers.h"
#include <iostream>
#include "ChunkGenerator.h"

int main()
{
	/*initBiomes();
	std::cout << "hello";
	LayerStack stack;

	stack = setupGenerator(MC_1_15);
	applySeed(&stack, 8675309);
	int* map = allocCache(&stack.layers[L_VORONOI_ZOOM_1], 1, 1);
	genArea(&stack.layers[L_VORONOI_ZOOM_1], map, -6176, 2611, 1, 1);

	std::cout << "Biome: " << *map << std::endl;*/

	std::cout<< std::numeric_limits<double>::is_iec559 << std::endl;

	int64_t seed = 8675309;
	setSeed(&seed);
	std::cout << sizeof(long) << std::endl;

	ChunkGenerator generator(8675309LL, &seed);
	ChunkData chunk;
	generator.provideChunk(-11, -3, chunk);

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

	return 0;
}