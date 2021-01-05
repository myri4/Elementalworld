--prom = vec4:new()
--prom.a = 3
--prom2 = vec4:new()
--
----prom = prom = prom2 

--print("Hi from lua ".. prom.w)

grassBlock = Block:new()
grassBlock.id = 1
grassBlock.isCollidable = true
			 
			 
texCoords = {
	vec2:new(0, 1),
	vec2:new(1, 0),
	vec2:new(0, 0),
	vec2:new(0, 0),
	vec2:new(0, 0),
	vec2:new(0, 0)
}			 
AddBlock3("scripts/grassblock.lua", texCoords)
--AddBlock("scripts/grassblock.lua", grassBlock)