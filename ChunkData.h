#pragma once

class ChunkData
{
public:
	ChunkData() {};
	~ChunkData() {};

	int getBlock(int, int, int);
	void setBlock(int, int, int, int);

private:
	int blocks_[16][256][16];
};