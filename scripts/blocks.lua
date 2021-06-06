AddBlockScript("scripts/grassblock.lua")
AddBlockScript("scripts/dirt.lua")
AddBlockScript("scripts/stone.lua")
AddBlockScript("scripts/glass.lua")
AddBlockScript("scripts/sand.lua")
AddBlockScript("scripts/water.lua")
AddBlockScript("scripts/wood.lua")
AddBlockScript("scripts/snow.lua")
AddBlockScript("scripts/leaves.lua")
AddBlockScript("scripts/flower.lua")
AddBlockScript("scripts/coal.lua")
AddBlockScript("scripts/haybale.lua")

Suit = {
    SPADES=1,
    HEARTS=2,
    DIAMONDS=3,
    CLUBS=4
}

grassblock = Block:new()
--grassblock.blockConnectionType = CONNECT_DEFAULT