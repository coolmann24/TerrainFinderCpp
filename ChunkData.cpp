#include "ChunkData.h"

int ChunkData::getBlock(int x, int y, int z)
{
	return blocks_[x][y][z];
}
void ChunkData::setBlock(int x, int y, int z, int block)
{
	blocks_[x][y][z] = block;
}

int ChunkHeightmap::getHeightmap(int x, int z)
{
	return heightmap_[x][z];
}
void ChunkHeightmap::setHeightmap(int x, int z, int height)
{
	heightmap_[x][z] = height;
}
int ChunkHeightmap::getBlock(int x, int y, int z) // 1 == terrain, 0 == air
{
	return heightmap_[x][z] == y ? 1 : 0;
}