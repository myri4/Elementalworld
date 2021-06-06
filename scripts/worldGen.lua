noise = Noise:new()
noise.octaves = 9 -- min 1 max 9
noise.scale = 90
noise.multiplier = 64
noise.persistance = 0.3
noise.lacunarity = 2
noise.seed = 2895

TempNoise = Noise:new()
TempNoise.octaves = 1 -- min 1 max 9
TempNoise.scale = 120
TempNoise.multiplier = 48
TempNoise.persistance = 0.4
TempNoise.lacunarity = 1.5
TempNoise.seed = noise.seed

water_level = 48
snow_level = 64


-- multiplier = 112