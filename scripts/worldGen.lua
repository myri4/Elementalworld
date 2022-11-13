noise = Noise:new()
noise:SetNoiseType(0)
noise:SetFractalType(1)
noise:SetOctaves(9)
noise:SetMultiplier(256)
noise:SetFrequency(1 / 2000) -- scale 
noise:SetLacunarity(2)
noise:SetGain(0.53) -- persistance, roughness

TreeNoise = Noise:new()
TreeNoise:SetNoiseType(3)
TreeNoise:SetFractalType(2)
TreeNoise:SetOctaves(5)
TreeNoise:SetLacunarity(1)
TreeNoise:SetFrequency(1 / 3)
TreeNoise:SetGain(0.003)

water_level = 32