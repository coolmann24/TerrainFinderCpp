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

void terrainSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations)
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

	ChunkGenerator generator(seed, MC_1_12);
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
	delete chunk;
	delete chunkxp;
	delete chunkzp;
	delete chunkxzp;
}

void cachedTerrainSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations)
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

	ChunkGenerator generator(seed, MC_1_13);
	std::unordered_map<int64_t, std::pair<int, std::shared_ptr<ChunkData>>> chunk_cache; //int counts uses (4 total), once accessed 3 additional times delete from cache (except if chunk on search edge uses is probably 2)
	std::shared_ptr<ChunkData> chunk, chunkxp, chunkzp, chunkxzp;

	for (int x = xminc; x <= xmaxc; x++)
	{
		for (int z = zminc; z <= zmaxc; z++)
		{
			int64_t xlong = x, zlong = z;
			int64_t key = (xlong << 32) + zlong, keyxp = ((xlong + 1) << 32) + zlong, keyzp = (xlong << 32) + (zlong + 1), keyxzp = ((xlong + 1) << 32) + (zlong + 1);

			if (std::end(chunk_cache) == chunk_cache.find(key))
			{
				chunk = std::make_shared<ChunkData>();
				generator.provideChunk(x, z, *chunk);
				chunk_cache[key] = std::make_pair(x > xminc && z > zminc ? 2 : 0, chunk);
			}
			else
			{
				chunk = chunk_cache[key].second;
				if (chunk_cache[key].first == 0)
					chunk_cache.erase(key);
				else
					chunk_cache[key].first--;
			}
			if (std::end(chunk_cache) == chunk_cache.find(keyxp))
			{
				chunkxp = std::make_shared<ChunkData>();
				generator.provideChunk(x + 1, z, *chunkxp);
				chunk_cache[keyxp] = std::make_pair((x + 1) < xmaxc && z > zminc ? 2 : 0, chunkxp);// if the chunk is on the edge of search we won't access it 4 times total but probably just twice
			}
			else
			{
				chunkxp = chunk_cache[keyxp].second;
				if (chunk_cache[keyxp].first == 0)
					chunk_cache.erase(keyxp);
				else
					chunk_cache[keyxp].first--;
			}
			if (std::end(chunk_cache) == chunk_cache.find(keyzp))
			{
				chunkzp = std::make_shared<ChunkData>();
				generator.provideChunk(x, z + 1, *chunkzp);
				chunk_cache[keyzp] = std::make_pair(x > xminc && (z + 1) < zmaxc ? 2 : 0, chunkzp);
			}
			else
			{
				chunkzp = chunk_cache[keyzp].second;
				if (chunk_cache[keyzp].first == 0)
					chunk_cache.erase(keyzp);
				else
					chunk_cache[keyzp].first--;
			}
			if (std::end(chunk_cache) == chunk_cache.find(keyxzp))
			{
				chunkxzp = std::make_shared<ChunkData>();
				generator.provideChunk(x + 1, z + 1, *chunkxzp);
				chunk_cache[keyxzp] = std::make_pair((x + 1) < xmaxc && (z + 1) < zmaxc ? 2 : 0, chunkxzp);
			}
			else
			{
				chunkxzp = chunk_cache[keyxzp].second;
				if (chunk_cache[keyxzp].first == 0)
					chunk_cache.erase(keyxzp);
				else
					chunk_cache[keyxzp].first--;
			}

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

		//std::cout << chunk_cache.size() << std::endl;
	}
}

void threadedTerrainSearch(TerrainSearchFunc func, int numThreads, int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, std::unordered_set<int> biomes, bool allRotations)
{
	if (numThreads < 0)
		throw std::runtime_error("Search threads must be greater than zero");

	if (xminc > xmaxc || ymin > ymax || zminc > zmaxc)
		throw std::runtime_error("Search range min bounds must be less than max");

	std::cout << "Searching with " << numThreads << " threads..." << std::endl;

	auto start = std::chrono::system_clock::now();

	std::unordered_set<int>* biomesptr = nullptr;
	if (biomes.size() > 0)
		biomesptr = &biomes;

	int width = xmaxc - xminc;
	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads - 1; i++)
	{
		threads.emplace_back(func, seed, xminc + i * (width / numThreads), xminc + (i + 1) * (width / numThreads) - 1, ymin, ymax, zminc, zmaxc, formation, biomesptr, allRotations);
	}
	threads.emplace_back(func, seed, xminc + (numThreads - 1) * (width / numThreads), xmaxc, ymin, ymax, zminc, zmaxc, formation, biomesptr, allRotations);

	for (size_t i = 0; i < threads.size(); i++)
		threads[i].join();

	std::cout << "Search completed in " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() << " seconds." << std::endl;
}


void heightmapSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, int biome, bool allRotations)
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

	ChunkGenerator generator(seed, MC_1_12);
	ChunkHeightmap* chunk = new ChunkHeightmap();
	ChunkHeightmap* chunkxp = new ChunkHeightmap();
	ChunkHeightmap* chunkzp = new ChunkHeightmap();
	ChunkHeightmap* chunkxzp = new ChunkHeightmap();

	auto base_and_variation = std::make_pair(generator.getBaseHeight(biome), generator.getHeightVariation(biome));

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
				generator.provideChunkHeightmap(x, z, *chunk, base_and_variation);
				generator.provideChunkHeightmap(x + 1, z, *chunkxp, base_and_variation);
			}
			generator.provideChunkHeightmap(x, z + 1, *chunkzp, base_and_variation);
			generator.provideChunkHeightmap(x + 1, z + 1, *chunkxzp, base_and_variation);

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
	delete chunk;
	delete chunkxp;
	delete chunkzp;
	delete chunkxzp;
}

void cachedHeightmapSearch(int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, int biome, bool allRotations)
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

	ChunkGenerator generator(seed, MC_1_13);
	std::unordered_map<int64_t, std::pair<int, std::shared_ptr<ChunkHeightmap>>> chunk_cache; //int counts uses (4 total), once accessed 3 additional times delete from cache (except if chunk on search edge uses is probably 2)
	std::shared_ptr<ChunkHeightmap> chunk, chunkxp, chunkzp, chunkxzp;

	auto base_and_variation = std::make_pair(generator.getBaseHeight(biome), generator.getHeightVariation(biome));

	for (int x = xminc; x <= xmaxc; x++)
	{
		for (int z = zminc; z <= zmaxc; z++)
		{
			int64_t xlong = x, zlong = z;
			int64_t key = (xlong << 32) + zlong, keyxp = ((xlong + 1) << 32) + zlong, keyzp = (xlong << 32) + (zlong + 1), keyxzp = ((xlong + 1) << 32) + (zlong + 1);

			if (std::end(chunk_cache) == chunk_cache.find(key))
			{
				chunk = std::make_shared<ChunkHeightmap>();
				generator.provideChunkHeightmap(x, z, *chunk, base_and_variation);
				chunk_cache[key] = std::make_pair(x > xminc && z > zminc ? 2 : 0, chunk);
			}
			else
			{
				chunk = chunk_cache[key].second;
				if (chunk_cache[key].first == 0)
					chunk_cache.erase(key);
				else
					chunk_cache[key].first--;
			}
			if (std::end(chunk_cache) == chunk_cache.find(keyxp))
			{
				chunkxp = std::make_shared<ChunkHeightmap>();
				generator.provideChunkHeightmap(x + 1, z, *chunkxp, base_and_variation);
				chunk_cache[keyxp] = std::make_pair((x + 1) < xmaxc && z > zminc ? 2 : 0, chunkxp);// if the chunk is on the edge of search we won't access it 4 times total but probably just twice
			}
			else
			{
				chunkxp = chunk_cache[keyxp].second;
				if (chunk_cache[keyxp].first == 0)
					chunk_cache.erase(keyxp);
				else
					chunk_cache[keyxp].first--;
			}
			if (std::end(chunk_cache) == chunk_cache.find(keyzp))
			{
				chunkzp = std::make_shared<ChunkHeightmap>();
				generator.provideChunkHeightmap(x, z + 1, *chunkzp, base_and_variation);
				chunk_cache[keyzp] = std::make_pair(x > xminc && (z + 1) < zmaxc ? 2 : 0, chunkzp);
			}
			else
			{
				chunkzp = chunk_cache[keyzp].second;
				if (chunk_cache[keyzp].first == 0)
					chunk_cache.erase(keyzp);
				else
					chunk_cache[keyzp].first--;
			}
			if (std::end(chunk_cache) == chunk_cache.find(keyxzp))
			{
				chunkxzp = std::make_shared<ChunkHeightmap>();
				generator.provideChunkHeightmap(x + 1, z + 1, *chunkxzp, base_and_variation);
				chunk_cache[keyxzp] = std::make_pair((x + 1) < xmaxc && (z + 1) < zmaxc ? 2 : 0, chunkxzp);
			}
			else
			{
				chunkxzp = chunk_cache[keyxzp].second;
				if (chunk_cache[keyxzp].first == 0)
					chunk_cache.erase(keyxzp);
				else
					chunk_cache[keyxzp].first--;
			}

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

		//std::cout << chunk_cache.size() << std::endl;
	}
}

void threadedHeightmapSearch(HeightmapSearchFunc func, int numThreads, int64_t seed, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock>& formation, int biome, bool allRotations)
{
	if (numThreads < 0)
		throw std::runtime_error("Search threads must be greater than zero");

	if (xminc > xmaxc || ymin > ymax || zminc > zmaxc)
		throw std::runtime_error("Search range min bounds must be less than max");

	std::cout << "Searching with " << numThreads << " threads..." << std::endl;

	auto start = std::chrono::system_clock::now();

	int width = xmaxc - xminc;
	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads - 1; i++)
	{
		threads.emplace_back(func, seed, xminc + i * (width / numThreads), xminc + (i + 1) * (width / numThreads) - 1, ymin, ymax, zminc, zmaxc, formation, biome, allRotations);
	}
	threads.emplace_back(func, seed, xminc + (numThreads - 1) * (width / numThreads), xmaxc, ymin, ymax, zminc, zmaxc, formation, biome, allRotations);

	for (size_t i = 0; i < threads.size(); i++)
		threads[i].join();

	std::cout << "Search completed in " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() << " seconds." << std::endl;
}