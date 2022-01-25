local FractalType = {
    None = 0,
    FBm = 1,
    Ridged = 2, -- sharp values(beautifull for Mountains tops)
    PingPong = 3 -- same as ridged
}

local NoiseType =
{
    OpenSimplex2 = 0, -- Dense
    OpenSimplex2S = 1, -- Not dense land
    Cellular = 2, -- more tests needed
    Perlin = 3, -- Good for biomes
    ValueCubic = 4, -- Mountains, big islands
    Value = 5 -- sharper Mountains
}

seed = 2895
noise = Noise:new()
frequency = 2000
noise:SetNoiseType(NoiseType.Ridged)
noise:SetFractalType(FractalType.FBm)
noise:SetOctaves(9)
noise:SetMultiplier(256)
noise:SetFrequency(1 / frequency) -- scale 
noise:SetLacunarity(2)
noise:SetGain(0.53) -- persistance, roughness
noise:SetSeed(seed)

TempNoise = Noise:new()
TempNoise:SetNoiseType(NoiseType.Ridged)
TempNoise:SetFractalType(FractalType.FBm)
TempNoise:SetOctaves(3)
TempNoise:SetSeed(seed)
TempNoise:SetLacunarity(2)
TempNoise:SetFrequency(1 / frequency)
TempNoise:SetGain(frequency * 0.001)
TempNoise:SetMultiplier(100)

MoistureNoise = Noise:new()
MoistureNoise:SetNoiseType(NoiseType.Ridged)
MoistureNoise:SetFractalType(FractalType.FBm)
MoistureNoise:SetOctaves(3)
MoistureNoise:SetSeed(seed)
MoistureNoise:SetLacunarity(2)
MoistureNoise:SetFrequency(1 / frequency)
MoistureNoise:SetGain(frequency * 0.001)

TreeNoise = Noise:new()
TreeNoise:SetNoiseType(NoiseType.Perlin)
TreeNoise:SetFractalType(FractalType.Ridged)
TreeNoise:SetOctaves(5)
TreeNoise:SetSeed(seed)
TreeNoise:SetLacunarity(1)
TreeNoise:SetFrequency(1 / 3)
TreeNoise:SetGain(3 * 0.001)

CaveNoise = Noise:new()
CaveNoise:SetOctaves(1)
CaveNoise:SetFrequency(1 / 90)
CaveNoise:SetMultiplier(5 * 0)
CaveNoise:SetFractalType(FractalType.Ridged)
CaveNoise:SetSeed(seed)
CaveNoise:SetNoiseType(NoiseType.OpenSimplex2S)

water_level = 32