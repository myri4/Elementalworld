plains = Biome:new()
plains.topBlock = 1
plains.minTemp = 10
plains.maxTemp = 40

desert = Biome:new()
desert.topBlock = 4
desert.minTemp = 41
desert.maxTemp = 100

snowPeak = Biome:new()
snowPeak.topBlock = 8
snowPeak.minTemp = -100
snowPeak.maxTemp = 9

AddBiome(plains)
AddBiome(desert)
AddBiome(snowPeak)

--//if (floraGen <= 0.4f && heightMap > water_level) { setBlock(glm::ivec3(x, y, z), 14, chunk); }
--//if (floraGen > 0.69f && floraGen <= 0.7f && heightMap > water_level) { setBlock(glm::ivec3(x, y, z), 13, chunk); }
--//if (floraGen > 0.59f && floraGen <= 0.6f && heightMap > water_level) { setBlock(glm::ivec3(x, y, z), 10, chunk); }
--//if (biomeMap[type].numberFloraBlocks > 0) {
--//	for (uint8_t i = 0; i < biomeMap[type].numberFloraBlocks; i++)
--//		if (biomeMap[type].floraTables[i].start <= floraGen &&
--//			biomeMap[type].floraTables[i].end > floraGen) {
--//			setBlock(glm::ivec3(x, y, z), biomeMap[type].floraTables[i].block, chunk);
--//		}
--//}