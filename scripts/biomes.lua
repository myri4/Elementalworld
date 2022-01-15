plains = Biome:new()
plains.trees = true
plains.topBlock = 1
plains.minTemp = 0.21
plains.maxTemp = 0.6
--plains:addFloraBlock(10)
--plains:addFloraBlock(14)
--plains:addFloraBlock(13)

desert = Biome:new()
desert.topBlock = 4
desert.trees = false
desert.minTemp = 0.61
desert.maxTemp = 1

snowPeak = Biome:new()
snowPeak.trees = false
snowPeak.topBlock = 8
snowPeak.minTemp = -1
snowPeak.maxTemp = 0.2

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