#include "Search.h"
#include <thread>

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

void terrainSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback* progresscb, FoundCallback foundcb)
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

	ChunkGenerator generator(seed, version);
	std::pair<bool, std::shared_ptr<ChunkData>> chunk = std::make_pair(false, std::make_shared<ChunkData>());
	std::pair<bool, std::shared_ptr<ChunkData>> chunkxp = std::make_pair(false, std::make_shared<ChunkData>());
	std::pair<bool, std::shared_ptr<ChunkData>> chunkzp = std::make_pair(false, std::make_shared<ChunkData>());
	std::pair<bool, std::shared_ptr<ChunkData>> chunkxzp = std::make_pair(false, std::make_shared<ChunkData>());

	for (int x = xminc; x <= xmaxc; x++)
	{
		if (keep_searching)
			if (!(*keep_searching))
				return;
		if (progresscb)
			(*progresscb)(static_cast<double>(x - xminc) / static_cast<double>(xmaxc - xminc));
		for (int z = zminc; z <= zmaxc; z++)
		{
			if (z != zminc)//reduce times we need to generate the chunk to 2 instead of 4
			{
				std::swap(chunk, chunkzp);
				std::swap(chunkxp, chunkxzp);
			}
			else
			{
				chunk.first = generator.provideChunk(x, z, *chunk.second, biomes);
				chunkxp.first = generator.provideChunk(x + 1, z, *chunkxp.second, biomes);
			}
			chunkzp.first = generator.provideChunk(x, z + 1, *chunkzp.second, biomes);
			chunkxzp.first = generator.provideChunk(x + 1, z + 1, *chunkxzp.second, biomes);

			if (!chunk.first || !chunkxp.first || !chunkzp.first || !chunkxzp.first)
				continue;

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
									if (chunkxzp.second->getBlock(xrel - 16, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (xrel >= 16)
								{
									if (chunkxp.second->getBlock(xrel - 16, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (zrel >= 16)
								{
									if (chunkzp.second->getBlock(xrel, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else
								{
									if (chunk.second->getBlock(xrel, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
							}
							if (found)
								foundcb(std::make_pair(x * 16 + xb, z * 16 + zb));
						}
					}
				}
			}
		}
	}
}

void cachedTerrainSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback* progresscb, FoundCallback foundcb)
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

	ChunkGenerator generator(seed, version);
	//std::unordered_map<int64_t, bool> valid_map;
	std::unordered_map<int64_t, std::pair<int, std::pair<bool, std::shared_ptr<ChunkData>>>> chunk_cache; //int counts uses (4 total), once accessed 3 additional times delete from cache (except if chunk on search edge uses is probably 2)
	std::pair<bool, std::shared_ptr<ChunkData>> chunk, chunkxp, chunkzp, chunkxzp;

	for (int x = xminc; x <= xmaxc; x++)
	{
		if (keep_searching)
			if (!(*keep_searching))
				return;
		if (progresscb)
			(*progresscb)(static_cast<double>(x - xminc) / static_cast<double>(xmaxc - xminc));
		for (int z = zminc; z <= zmaxc; z++)
		{
			int64_t xlong = x, zlong = z;
			int64_t key = (xlong << 32) + zlong, keyxp = ((xlong + 1) << 32) + zlong, keyzp = (xlong << 32) + (zlong + 1), keyxzp = ((xlong + 1) << 32) + (zlong + 1);

			if (std::end(chunk_cache) == chunk_cache.find(key))
			{
				auto ch = std::make_shared<ChunkData>();
				bool valid = generator.provideChunk(x, z, *ch, biomes);
				chunk_cache[key] = std::make_pair(x > xminc && z > zminc ? 2 : 0, chunk = std::make_pair(valid, ch));
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
				auto ch = std::make_shared<ChunkData>();
				bool valid = generator.provideChunk(x + 1, z, *ch, biomes);
				chunk_cache[keyxp] = std::make_pair((x + 1) < xmaxc && z > zminc ? 2 : 0, chunkxp = std::make_pair(valid, ch));// if the chunk is on the edge of search we won't access it 4 times total but probably just twice
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
				auto ch = std::make_shared<ChunkData>();
				bool valid = generator.provideChunk(x, z + 1, *ch, biomes);
				chunk_cache[keyzp] = std::make_pair(x > xminc && (z + 1) < zmaxc ? 2 : 0, chunkzp = std::make_pair(valid, ch));
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
				auto ch = std::make_shared<ChunkData>();
				bool valid = generator.provideChunk(x + 1, z + 1, *ch, biomes);
				chunk_cache[keyxzp] = std::make_pair((x + 1) < xmaxc && (z + 1) < zmaxc ? 2 : 0, chunkxzp = std::make_pair(valid, ch));
			}
			else
			{
				chunkxzp = chunk_cache[keyxzp].second;
				if (chunk_cache[keyxzp].first == 0)
					chunk_cache.erase(keyxzp);
				else
					chunk_cache[keyxzp].first--;
			}

			if (!chunk.first || !chunkxp.first || !chunkzp.first || !chunkxzp.first) //biome mismatch of any chunk
				continue;

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
									if (chunkxzp.second->getBlock(xrel - 16, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (xrel >= 16)
								{
									if (chunkxp.second->getBlock(xrel - 16, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (zrel >= 16)
								{
									if (chunkzp.second->getBlock(xrel, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else
								{
									if (chunk.second->getBlock(xrel, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
							}
							if (found)
								foundcb(std::make_pair(x * 16 + xb, z * 16 + zb));
						}
					}
				}
			}
		}
	}
}


void heightmapSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback* progresscb, FoundCallback foundcb)
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

	ChunkGenerator generator(seed, version);
	std::pair<bool, std::shared_ptr<ChunkHeightmap>> chunk = std::make_pair(false, std::make_shared<ChunkHeightmap>());
	std::pair<bool, std::shared_ptr<ChunkHeightmap>> chunkxp = std::make_pair(false, std::make_shared<ChunkHeightmap>());
	std::pair<bool, std::shared_ptr<ChunkHeightmap>> chunkzp = std::make_pair(false, std::make_shared<ChunkHeightmap>());
	std::pair<bool, std::shared_ptr<ChunkHeightmap>> chunkxzp = std::make_pair(false, std::make_shared<ChunkHeightmap>());

	//auto base_and_variation = std::make_pair(generator.getBaseHeight(biome), generator.getHeightVariation(biome));

	for (int x = xminc; x <= xmaxc; x++)
	{
		if (keep_searching)
			if (!(*keep_searching))
				return;
		if (progresscb)
			(*progresscb)(static_cast<double>(x - xminc) / static_cast<double>(xmaxc - xminc));
		for (int z = zminc; z <= zmaxc; z++)
		{
			if (z != zminc)//reduce times we need to generate the chunk to 2 instead of 4
			{
				std::swap(chunk, chunkzp);
				std::swap(chunkxp, chunkxzp);
			}
			else
			{
				chunk.first = generator.provideChunkHeightmap(x, z, *chunk.second, biomes);
				chunkxp.first = generator.provideChunkHeightmap(x + 1, z, *chunkxp.second, biomes);
			}
			chunkzp.first = generator.provideChunkHeightmap(x, z + 1, *chunkzp.second, biomes);
			chunkxzp.first = generator.provideChunkHeightmap(x + 1, z + 1, *chunkxzp.second, biomes);

			if (!chunk.first || !chunkxp.first || !chunkzp.first || !chunkxzp.first)
				continue;

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
									if (chunkxzp.second->getBlock(xrel - 16, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (xrel >= 16)
								{
									if (chunkxp.second->getBlock(xrel - 16, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (zrel >= 16)
								{
									if (chunkzp.second->getBlock(xrel, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else
								{
									if (chunk.second->getBlock(xrel, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
							}
							if (found)
								foundcb(std::make_pair(x * 16 + xb, z * 16 + zb));
						}
					}
				}
			}
		}
	}
}

void cachedHeightmapSearch(uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int>* biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback* progresscb, FoundCallback foundcb)
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

	ChunkGenerator generator(seed, version);
	std::unordered_map<uint64_t, std::pair<int, std::pair<bool, std::shared_ptr<ChunkHeightmap>>>> chunk_cache; //int counts uses (4 total), once accessed 3 additional times delete from cache (except if chunk on search edge uses is probably 2)
	std::pair<bool, std::shared_ptr<ChunkHeightmap>> chunk, chunkxp, chunkzp, chunkxzp;

	for (int x = xminc; x <= xmaxc; x++)
	{
		if (keep_searching)
			if (!(*keep_searching))
				return;
		if (progresscb)
			(*progresscb)(static_cast<double>(x - xminc) / static_cast<double>(xmaxc - xminc));
		for (int z = zminc; z <= zmaxc; z++)
		{
			int64_t xlong = x, zlong = z;
			int64_t key = (xlong << 32) + zlong, keyxp = ((xlong + 1) << 32) + zlong, keyzp = (xlong << 32) + (zlong + 1), keyxzp = ((xlong + 1) << 32) + (zlong + 1);

			if (std::end(chunk_cache) == chunk_cache.find(key))
			{
				auto ch = std::make_shared<ChunkHeightmap>();
				bool valid = generator.provideChunkHeightmap(x, z, *ch, biomes);
				chunk_cache[key] = std::make_pair(x > xminc && z > zminc ? 2 : 0, chunk = std::make_pair(valid, ch));
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
				auto ch = std::make_shared<ChunkHeightmap>();
				bool valid = generator.provideChunkHeightmap(x + 1, z, *ch, biomes);
				chunk_cache[keyxp] = std::make_pair((x + 1) < xmaxc && z > zminc ? 2 : 0, chunkxp = std::make_pair(valid, ch));// if the chunk is on the edge of search we won't access it 4 times total but probably just twice
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
				auto ch = std::make_shared<ChunkHeightmap>();
				bool valid = generator.provideChunkHeightmap(x, z + 1, *ch, biomes);
				chunk_cache[keyzp] = std::make_pair(x > xminc && (z + 1) < zmaxc ? 2 : 0, chunkzp = std::make_pair(valid, ch));
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
				auto ch = std::make_shared<ChunkHeightmap>();
				bool valid = generator.provideChunkHeightmap(x + 1, z + 1, *ch, biomes);
				chunk_cache[keyxzp] = std::make_pair((x + 1) < xmaxc && (z + 1) < zmaxc ? 2 : 0, chunkxzp = std::make_pair(valid, ch));
			}
			else
			{
				chunkxzp = chunk_cache[keyxzp].second;
				if (chunk_cache[keyxzp].first == 0)
					chunk_cache.erase(keyxzp);
				else
					chunk_cache[keyxzp].first--;
			}

			if (!chunk.first || !chunkxp.first || !chunkzp.first || !chunkxzp.first) //biome mismatch of any chunk
				continue;

			for (int xb = 0; xb < 16; xb++)
			{
				for (int zb = 0; zb < 16; zb++)
				{
					/*for (int y = ymin; y <= ymax; y++)
					{*/
						for (auto& form : formations)
						{
							int chunky;

							int xrelinit = xb + form[0].getX();
							int zrelinit = zb + form[0].getZ();

							if (xrelinit >= 16 && zrelinit >= 16)
							{
								chunky = chunkxzp.second->getHeightmap(xrelinit - 16, zrelinit - 16);
							}
							else if (xrelinit >= 16)
							{
								chunky = chunkxp.second->getHeightmap(xrelinit - 16, zrelinit);
							}
							else if (zrelinit >= 16)
							{
								chunky = chunkzp.second->getHeightmap(xrelinit, zrelinit - 16);
							}
							else
							{
								chunky = chunk.second->getHeightmap(xrelinit, zrelinit);
							}

							int y = chunky - form[0].getY();
						
							bool found = true;
							for (auto& block : form)
							{
								int xrel = xb + block.getX();
								int zrel = zb + block.getZ();

								if (xrel >= 16 && zrel >= 16)
								{
									if (chunkxzp.second->getBlock(xrel - 16, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (xrel >= 16)
								{
									if (chunkxp.second->getBlock(xrel - 16, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else if (zrel >= 16)
								{
									if (chunkzp.second->getBlock(xrel, y + block.getY(), zrel - 16) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
								else
								{
									if (chunk.second->getBlock(xrel, y + block.getY(), zrel) != block.getBlockId())
									{
										found = false;
										break;
									}
								}
							}
							if (found)
								foundcb(std::make_pair(x * 16 + xb, z * 16 + zb));
						}
					//}
				}
			}
		}
	}
}

void threadedSearch(SearchFunc func, int numThreads, uint64_t seed, MCversion version, int xminc, int xmaxc, int ymin, int ymax, int zminc, int zmaxc, std::vector<FormationBlock> formation, std::unordered_set<int> biomes, bool allRotations, std::atomic_bool* keep_searching, ProgressCallback* progresscb, FoundCallback foundcb, FinishedCallback finishedcb)
{
	if (numThreads < 0)
		throw std::runtime_error("Search threads must be greater than zero");

	if (xminc > xmaxc || ymin > ymax || zminc > zmaxc)
		throw std::runtime_error("Search range min bounds must be less than max");

	std::unordered_set<int>* biomesptr = nullptr;
	if (biomes.size() > 0)
		biomesptr = &biomes;

	int width = xmaxc - xminc;
	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads - 1; i++)
	{
		threads.emplace_back(func, seed, version, xminc + i * (width / numThreads), xminc + (i + 1) * (width / numThreads) - 1, ymin, ymax, zminc, zmaxc, formation, biomesptr, allRotations, keep_searching, nullptr, foundcb);
	}
	threads.emplace_back(func, seed, version, xminc + (numThreads - 1) * (width / numThreads), xmaxc, ymin, ymax, zminc, zmaxc, formation, biomesptr, allRotations, keep_searching, progresscb, foundcb);

	for (size_t i = 0; i < threads.size(); i++)
		threads[i].join();

	finishedcb();
}