#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <unordered_map>
#include "files.hpp"
#include <cassert>
#include "Chunk.hpp"

#define REGION_SIZE_BIT 5
#define REGION_SIZE (1 << REGION_SIZE_BIT)
#define REGION_VOL	(REGION_SIZE * REGION_SIZE)

/* Требование:
 * - высота мира = 1 чанк (любых размеров)
 * Пример:
 * - CHUNK_W = 16, CHUNK_H = 128, CHUNK_D = 16
 * */

union {
	long _key;
	int _coords[3];
} _tempcoords;

int bytes2Int(const uint8_t* src, uint32_t offset) {
	return (src[offset] << 24) | (src[offset + 1] << 16) | (src[offset + 2] << 8) | (src[offset + 3]);
}

void int2Bytes(int value, char* dest, uint32_t offset) {
	dest[offset] = (char)(value >> 24 & 255);
	dest[offset + 1] = (char)(value >> 16 & 255);
	dest[offset + 2] = (char)(value >> 8 & 255);
	dest[offset + 3] = (char)(value >> 0 & 255);
}

class WorldFiles {
public:
	std::unordered_map<long, char**> regions;
	std::string directory;
	char* mainBuffer;

	WorldFiles(const char* dir, size_t mainBufferCapacity) : directory(dir) {
		mainBuffer = new char[mainBufferCapacity];
	}

	~WorldFiles() {
		delete[] mainBuffer;
		std::unordered_map<long, char**>::iterator it;
		for (it = regions.begin(); it != regions.end(); it++) {
			char** region = it->second;
			if (region == nullptr)
				continue;
			for (uint32_t i = 0; i < REGION_VOL; i++) 
				delete[] region[i];
			
			delete[] region;
		}
		regions.clear();
	}

	void put(const char* chunkData, int x, int y, int z) {
		assert(chunkData != nullptr);

		int regionX = x >> REGION_SIZE_BIT;
		int regionY = y >> REGION_SIZE_BIT;
		int regionZ = z >> REGION_SIZE_BIT;

		int localX = x - (regionX << REGION_SIZE_BIT);
		int localY = y - (regionY << REGION_SIZE_BIT);
		int localZ = z - (regionZ << REGION_SIZE_BIT);

		_tempcoords._coords[0] = regionX;
		_tempcoords._coords[1] = regionY;
		_tempcoords._coords[2] = regionZ;
		char** region = regions[_tempcoords._key];
		if (region == nullptr) {
			region = new char* [REGION_VOL];
			for (uint32_t i = 0; i < REGION_VOL; i++)
				region[i] = nullptr;
			regions[_tempcoords._key] = region;
		}
		char* targetChunk = region[localY * REGION_SIZE + localX];
		if (targetChunk == nullptr) {
			targetChunk = new char[chunkVolume];
			region[localY * REGION_SIZE + localX] = targetChunk;
		}
		for (uint32_t i = 0; i < chunkVolume; i++)
			targetChunk[i] = chunkData[i];
	}

	bool readChunk(int x, int y, int z, char* out) {
		assert(out != nullptr);

		int regionX = x >> REGION_SIZE_BIT;
		int regionY = y >> REGION_SIZE_BIT;
		int regionZ = z >> REGION_SIZE_BIT;

		int localX = x - (regionX << REGION_SIZE_BIT);
		int localY = y - (regionY << REGION_SIZE_BIT);
		int localZ = z - (regionZ << REGION_SIZE_BIT);
		int chunkIndex = localY * REGION_SIZE + localX;

		std::string filename = getRegionFile(regionX, regionY, regionZ);

		std::ifstream input(filename, std::ios::binary);
		if (!input.is_open()) 
			return false;
		

		uint32_t offset;
		input.seekg(chunkIndex * 4);
		input.read((char*)(&offset), 4);
		// Ordering bytes from big-endian to machine order (any, just reading)
		offset = bytes2Int((const uint8_t*)(&offset), 0);
		if (offset == 0) {
			input.close();
			return false;
		}

		input.seekg(offset);
		input.read((char*)(&offset), 4);
		size_t compressedSize = bytes2Int((const uint8_t*)(&offset), 0);

		input.read(mainBuffer, compressedSize);
		input.close();

		decompressRLE(mainBuffer, compressedSize, out, chunkVolume);

		return true;
	}

	bool getChunk(int x, int y, int z, char* out) {
		assert(out != nullptr);

		int regionX = x >> REGION_SIZE_BIT;
		int regionY = y >> REGION_SIZE_BIT;

		int localX = x - (regionX << REGION_SIZE_BIT);
		int localY = y - (regionY << REGION_SIZE_BIT);
		int chunkIndex = localY * REGION_SIZE + localX;
		assert(chunkIndex >= 0 && chunkIndex < REGION_VOL);

		_tempcoords._coords[0] = regionX;
		_tempcoords._coords[1] = regionY;

		char** region = regions[_tempcoords._key];
		if (region == nullptr)
			return readChunk(x, y, z, out);

		char* chunk = region[chunkIndex];
		if (chunk == nullptr)
			return readChunk(x, y, z, out);
		for (uint32_t i = 0; i < chunkVolume; i++)
			out[i] = chunk[i];
		return true;
	}
	//void readRegion(char* fileContent);
	uint32_t writeRegion(char* out, int x, int y, int z, char** region) {
		uint32_t offset = REGION_VOL * 4;
		for (uint32_t i = 0; i < offset; i++)
			out[i] = 0;

		char* compressed = new char[chunkVolume * 2];
		for (int i = 0; i < REGION_VOL; i++) {
			char* chunk = region[i];
			if (chunk == nullptr) {
				chunk = new char[chunkVolume];
				if (readChunk((i % REGION_SIZE) + x * REGION_SIZE, (i / REGION_SIZE) + y * REGION_SIZE, z, chunk)) 
					region[i] = chunk;				
				else {
					delete[] chunk;
					chunk = nullptr;
				}
			}

			if (chunk == nullptr) 
				int2Bytes(0, out, i * 4);			
			else {
				int2Bytes(offset, out, i * 4);

				uint32_t compressedSize = compressRLE(chunk, chunkVolume, compressed);

				int2Bytes(compressedSize, out, offset);
				offset += 4;

				for (uint32_t j = 0; j < compressedSize; j++)
					out[offset++] = compressed[j];
			}
		}
		delete[] compressed;
		return offset;
	}

	void write() {
		std::unordered_map<long, char**>::iterator it;
		for (it = regions.begin(); it != regions.end(); it++) {
			if (it->second == nullptr)
				continue;

			int x;
			int y;
			int z;
			//longToCoords(x, y, it->first);

			_tempcoords._key = it->first;
			x = _tempcoords._coords[0];
			y = _tempcoords._coords[1];

			uint32_t size = writeRegion(mainBuffer, x, y, z, it->second);
			write_binary_file(getRegionFile(x, y, z), mainBuffer, size);
		}
	}

	std::string getRegionFile(int x, int y, int z) {
		return directory + "r." + std::to_string(x * cs) + "." + std::to_string(y * cs) + "." + std::to_string(z * cs) + ".ewr";
	}
};

#endif /* FILES_WORLDFILES_H_ */