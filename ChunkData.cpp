#include "ChunkData.h"

int ChunkData::getBlock(int x, int y, int z)
{
	return blocks_[x][y][z];
}
void ChunkData::setBlock(int x, int y, int z, int block)
{
	blocks_[x][y][z] = block;
}