local ConnectionType = { 
    CONNECT_DEFAULT = 0, 
    FLUID_CONNECT = 1, 
    NO_CONNECT = 2,
    X_CONNECT = 3, 
    CANT_CONNECT = 4
}

local FractalType = {
    None = 0,
    FBm = 1,
    Ridged = 2,
    PingPong = 3
}

local NoiseType =
{
    OpenSimplex2 = 0,
    OpenSimplex2S = 1,
    Cellular = 2,
    Perlin = 3,
    ValueCubic = 4,
    Value = 5
}