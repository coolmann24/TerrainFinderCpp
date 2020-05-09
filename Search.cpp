#include "Search.h"

void alignFormation(std::vector<FormationBlock>& formation)
{
	int minx = INT_MAX, minz = INT_MAX;
	for (auto& block : formation)
	{
		if (block.getX() < minx) minx = block.getX();
		if (block.getZ() < minz) minz = block.getZ();
	}
	std::vector<FormationBlock> aligned_formation;
	for (auto& block : formation)
		aligned_formation.push_back(FormationBlock(block.getX() - minx, block.getY(), block.getZ() - minz, block.getBlockId()));
	for (auto& block : aligned_formation)
		if (block.getX() >= 16 || block.getZ() >= 16)
			throw std::runtime_error("Search formation width a maximum of 16 blocks.");
	formation = aligned_formation;
}

void search(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int>* biomes, bool allRotations)
{
	if (xminc > xmaxc || ymin > ymax || zminc > zmaxc)
		throw std::runtime_error("Search range min bounds must be less than max");

	std::vector<std::vector<FormationBlock>> formations(0);
	formations.resize(1);

	formations[0] = formation;
	alignFormation(formations[0]);

	if (allRotations)
	{
		formations.resize(4);
		for (int i = 1; i < 4; i++)
		{
			for (FormationBlock block : formations[i - 1])
				formations[i].push_back(FormationBlock(-1 * block.getZ(), block.getY(), block.getX(), block.getBlockId()));
			alignFormation(formations[i]);
		}
	}

	ChunkGenerator generator(seed);
	ChunkData* chunk = new ChunkData();
	ChunkData* chunkxp = new ChunkData();
	ChunkData* chunkzp = new ChunkData();
	ChunkData* chunkxzp = new ChunkData();

	for (int x = xminc; x <= xmaxc; x++)
	{
		for (int z = zminc; z <= zmaxc; z++)
		{
			if (z != zminc)//reduce times we need to generate the chunk to 2 instead of 4
			{
				std::swap(chunk, chunkzp);
				std::swap(chunkxp, chunkxzp);
			}
			else
			{
				generator.provideChunk(x, z, *chunk);
				generator.provideChunk(x + 1, z, *chunkxp);
			}
			generator.provideChunk(x, z + 1, *chunkzp);
			generator.provideChunk(x + 1, z + 1, *chunkxzp);

			for (int xb = 0; xb < 16; xb++)
			{
				for (int zb = 0; zb < 16; zb++)
				{
					for (int y = ymin; y <= ymax; y++)
					{
						for (auto& form : formations)
						{
							bool found = true;
							for (auto& block : form)
							{
								int xrel = xb + block.getX();
								int zrel = zb + block.getZ();

								if (xrel >= 16 && zrel >= 16)
								{
									if (chunkxzp->getBlock(xrel - 16, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (xrel >= 16)
								{
									if (chunkxp->getBlock(xrel - 16, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (zrel >= 16)
								{
									if (chunkzp->getBlock(xrel, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else
								{
									if (chunk->getBlock(xrel, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
							}
							if (found)
								std::cout << "X: " << (x * 16 + xb) << " Z: " << (z * 16 + zb) << std::endl;
						}
					}
				}
			}
		}
	}
}

void threadedSearch(int64_t seed, int numThreads, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int>* biomes, bool allRotations)
{
	if (numThreads < 0)
		throw std::runtime_error("Search threads must be greater than zero");
}