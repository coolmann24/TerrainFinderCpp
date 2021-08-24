#pragma once
#ifndef CHUNKDATA_H
#define CHUNKDATA_H

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

class ChunkHeightmap
{
public:
	ChunkHeightmap() {};
	~ChunkHeightmap() {};

	int getHeightmap(int, int);
	void setHeightmap(int, int, int);

	int getBlock(int, int, int);

private:
	int heightmap_[16][16];
};

#endif