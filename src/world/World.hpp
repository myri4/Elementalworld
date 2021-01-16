// Game idea
// Space ship game where you go around planets, gather resources then go and fight people an invade their spaceships, until you invde all the galaxy
// Type: strategy, fps

#ifndef WORLD_HPP
#define WORLD_HPP

#define GLM_FORCE_CTOR_INIT

#include "Chunk.hpp"
#include "Block.hpp"
#include "../entityes/Player.hpp"
#include <Utils/Time.hpp>
#include <Maths/Frustum.hpp>
#include <Maths/Noise.hpp>
#include <map>
#include "Biome.hpp"
#include <GUI/AssetManager.hpp>

namespace wc {

	static std::unordered_map<int, Block> blockData;
	static AssetManager assets;

	static void AddBlock(const char* script) {
		Block block;

		std::string conType;
		sol::state blockState;
		blockState.script_file(script);
		if (blockState["id"].valid()) block.id = blockState["id"];
		if (blockState["isCollidable"].valid()) block.isCollidable = blockState["isCollidable"];
		if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];

		if (conType == "CONNECT_DEFAULT") block.blockConnectionType = ConnectionType::CONNECT_DEFAULT;
		if (conType == "FLUID_CONNECT")   block.blockConnectionType = ConnectionType::FLUID_CONNECT;
		if (conType == "NO_CONNECT")      block.blockConnectionType = ConnectionType::NO_CONNECT;
		if (conType == "X_CONNECT")      block.blockConnectionType = ConnectionType::X_CONNECT;
		
		
		if (blockState["allTextures"].valid()) {
			block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(blockState["allTextures"]);
			block.texture[(int)BlockTexture::BOTTOM] = block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::FRONT] =  block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::BACK] =   block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::LEFT] =   block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::RIGHT] =  block.texture[(int)BlockTexture::TOP];
		}
		if(blockState["top"].valid())    block.texture[(int)BlockTexture::TOP] =    assets.LoadTexture(blockState["top"]);
		if(blockState["bottom"].valid()) block.texture[(int)BlockTexture::BOTTOM] = assets.LoadTexture(blockState["bottom"]);
		if(blockState["front"].valid())  block.texture[(int)BlockTexture::FRONT] =  assets.LoadTexture(blockState["front"]);
		if(blockState["back"].valid())   block.texture[(int)BlockTexture::BACK] =   assets.LoadTexture(blockState["back"]);
		if(blockState["left"].valid())   block.texture[(int)BlockTexture::LEFT] =   assets.LoadTexture(blockState["left"]);
		if(blockState["right"].valid())  block.texture[(int)BlockTexture::RIGHT] =  assets.LoadTexture(blockState["right"]);

		if (blockState["emitLight"].valid()) block.emitLight = blockState["emitLight"];

		blockData[block.id] = block;
	}

	static void LoadBlocks() {
		sol::state luaState;
		
		luaState.set_function("AddBlock", &AddBlock);
		luaState.open_libraries(sol::lib::base);
		//luaState.new_usertype<glm::vec2>("vec2", sol::constructors<void(), void(float, float), void(float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y);
		//
		//luaState.new_usertype<glm::vec3>("vec3", sol::constructors<void(), void(float, float, float), void(float)>(), "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z,
		//	"r", &glm::vec3::r, "g", &glm::vec3::g, "b", &glm::vec3::b);
		//luaState.new_usertype<glm::vec4>("vec4", sol::constructors<void(), void(float, float, float, float), void(float)>(), "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w,
		//	"r", &glm::vec4::r, "g", &glm::vec4::g, "b", &glm::vec4::b, "a", &glm::vec4::a);
		//
		//luaState.new_usertype<Block>("Block", sol::constructors<void(), void(const char*)>(), "id", &Block::id, "isCollidable", &Block::isCollidable);
		//
		luaState.script_file("scripts/blocks.lua");			
	}

	class Singleplayer : public NonCopyable {
	public:
		Player p;
		Biome defBiome;		

		Singleplayer() {}

		void Create() {
			worldShader.Create("shaderpacks/default/chunkShader.glsl");
			sol::state noiseState;
			noiseState.script_file("scripts/worldGen.lua");

			worldNoise.lacunarity = noiseState["lacunarity"];
			worldNoise.multiplier = noiseState["multiplier"];
			worldNoise.octaves = noiseState["octaves"];
			worldNoise.persistance = noiseState["persistance"];
			worldNoise.scale = noiseState["scale"];
			worldNoise.seed = noiseState["seed"];

			water_level = noiseState["water_level"];
			//snow_level = noiseState["snow_level"];

			//biomeNoise.lacunarity = 2;
			//biomeNoise.multiplier = 64;
			//biomeNoise.octaves = 2;
			//biomeNoise.persistance = 0.5;
			//biomeNoise.scale = 90;
			//biomeNoise.seed = 10;

			assets.Create(60, 32, 32);

			LoadBlocks();			

			p.InitPlayer({ chunkSize * chunkSize / 2 + chunkSize,chunkSize * 4,chunkSize * chunkSize / 2 });

			for (ChunkID i = 0; i < world.size(); i++) {
				world[i].chunkPos = to3D(i);
				//Configuring the vertex array
				world[i].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				world[i].chunkMeshArray.Create();
				world[i].chunkMeshArray.VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				world[i].chunkMeshArray.VertexAttribPointer(1, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
				world[i].chunkMeshArray.VertexAttribPointer(2, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal)); // normal coord attribute
				world[i].chunkMeshArray.VertexAttribPointer(3, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, type)); // type attribute
			}


			uint32_t indices[MaxFaceCount * 6];
			uint32_t ioffset = 0;
			for (uint32_t i = 0; i < sizeof(indices) / sizeof(uint32_t); i += 6) {
				indices[i + 0] = 0 + ioffset;
				indices[i + 1] = 1 + ioffset;
				indices[i + 2] = 2 + ioffset;

				indices[i + 3] = 2 + ioffset;
				indices[i + 4] = 3 + ioffset;
				indices[i + 5] = 0 + ioffset;

				ioffset += 4;
			}
			worldIndexBuffer.Create(indices, sizeof(indices));
			//WC_INFO("{0}, {1}, {2},", world[15].chunkMeshArray.GetRendererID(), world[15].chunkPos.y, world[15].chunkPos.z);
			//defBiome.Create("scripts/biomeTest.lua");
		}

		void Update(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {
			p.UpdatePlayer(windpos, windsize, CenterMouse, deltaTime);
	
			assets.Bind();

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.camera.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("u_Projection", p.projection);

			// passing the delta time variable to the shader
			worldShader.setFloat("deltaTime", deltaTime);

			getChunkID(glm::vec3(1));
			// camera/view transformation
			worldShader.setMat4("u_View", p.GetView());
			uint8_t chunkHalf = chunkSize / 2;
			for (ChunkID i = 0; i < world.size(); i++) {

				glm::vec3 currChunkPos = world[i].chunkPos;
				glm::vec3 pos = world[i].chunkPos * glm::vec3(chunkSize);

				glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

				if (world[i].IndexCount > 0) {
					world[i].chunkMeshArray.Bind();
					worldShader.setMat4("u_Model", model); // calculate the model matrix for each object and pass it to shader before drawing
					glDrawElements(GL_TRIANGLES, world[i].IndexCount, GL_UNSIGNED_INT, nullptr); // Drawing the cubes
				}				

				if (currChunkPos.x < glm::floor(p.Position.x / chunkSize) - chunkHalf) ResetChunk(i, glm::vec3(glm::floor(p.Position.x / chunkSize) + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > glm::floor(p.Position.x / chunkSize) + chunkHalf) ResetChunk(i, glm::vec3(glm::floor(p.Position.x / chunkSize) - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				if (currChunkPos.y < glm::floor(p.Position.y / chunkSize) - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, glm::floor(p.Position.y / chunkSize) - chunkHalf + 1, currChunkPos.z));
				if (currChunkPos.y > glm::floor(p.Position.y / chunkSize) + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, glm::floor(p.Position.y / chunkSize) + chunkHalf - 1, currChunkPos.z));

				if (currChunkPos.z < glm::floor(p.Position.z / chunkSize) - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, glm::floor(p.Position.z / chunkSize) + chunkHalf - 1));
				if (currChunkPos.z > glm::floor(p.Position.z / chunkSize) + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, glm::floor(p.Position.z / chunkSize) - chunkHalf + 1));

				// Updating the chunk`s mesh
				if (!world[i].generated) { GenerateChunkTerrain(i);	world[i].generated = true;	}
				if (world[i].canBeUpdated) { UpdateMesh(i);	world[i].canBeUpdated = false;	}
			}
		}

		void OnInput(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
			bool bBreak = wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON;
			bool bPlace = wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON;

			if (bBreak || bPlace) {
				glm::vec3 m_rayLastPos = glm::vec3(0.0f);
				Ray ray(p.Position);
				while (ray.getLength() < 4) {
					ray.Step(p.camera.Yaw, p.camera.Pitch, 0.05f);

					if (getBlock(ray.getEnd()) > 0) 
						if (bBreak) { 
							setBlock(glm::floor(ray.getEnd()), 0); break; }
						else if (bPlace) { setBlock(glm::floor(m_rayLastPos), p.ItemHolding); break; }
					
					m_rayLastPos = ray.getEnd();
				}
			}
		}

		//float waveTime = 1.3f;
	private:

		//Chunk managing

		void ResetChunk(const ChunkID& chunk, const glm::vec3& newChunkPos) {
			//wc::Timer timer("ResetChunk");
			world[chunk].chunkPos = newChunkPos;
			GenerateChunkTerrain(chunk);
			UpdateMesh(chunk);
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {

				memset(&world[chunk].chunkData, 0, sizeof(world[chunk].chunkData));
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap =
						worldNoise.getNoiseFor(
							x + world[chunk].chunkPos.x * chunkSize, 
							z + world[chunk].chunkPos.z * chunkSize);
						//int biomeMap = biomeNoise.getNoiseFor(
						//	x + world[chunk].chunkPos.x * chunkSize,
						//	z + world[chunk].chunkPos.z * chunkSize);
						for (uint8_t y = 0; y < chunkSize; y++) {
							glm::vec3 pos = world[chunk].chunkPos * glm::vec3(chunkSize) + glm::vec3(x, y, z);
							if (pos.y == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
							if (pos.y < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
							if (pos.y < heightMap - 3) { setBlock(glm::vec3(x, y, z), 3, chunk); } // @TODO randomnes
							if (pos.y == heightMap && pos.y <= water_level) { setBlock(glm::vec3(x, y, z), 4, chunk); } // @TODO randomnes
							if (pos.y > heightMap && pos.y < water_level) { setBlock(glm::vec3(x, y, z), 5, chunk); }
							//if (pos.y == heightMap && rand() % 100 > 98 && pos.y > water_level) setBlock(pos, 7); // @TODO randomnes
							//if (pos == heightMap && pos > snow_level) { setBlock(glm::vec3(x, y + 1, z), 8, chunk); }
							//if (pos == heightMap && heightMap > water_level && biomeMap > 48) { setBlock(glm::vec3(x, y, z), 4, chunk); }
						}
					}
		}

		void setBlock(const glm::vec3& pos, const BlockID& block, const ChunkID& chunk) {
			if (chunk < world.size() && chunk > 0) {
				int8_t x = glm::floor(pos.x);
				int8_t y = glm::floor(pos.y);
				int8_t z = glm::floor(pos.z);
				glm::vec3 blockPos = getBlockPos(pos);
				if (x >= chunkSize) x = blockPos.x;
				if (y >= chunkSize) y = blockPos.y;
				if (z >= chunkSize) z = blockPos.z;
				if (world[chunk].chunkData[x][y][z] != block) {
					world[chunk].chunkData[x][y][z] = block;
					world[chunk].canBeUpdated = true;

					//int32_t neighborXpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 1,0,0 });
					//int32_t neighborYpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,1,0 });
					//int32_t neighborZpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,0,1 });
					//
					//int32_t neighborXneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 1,0,0 });
					//int32_t neighborYneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,1,0 });
					//int32_t neighborZneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,0,1 });

					if (chunk - to1D({ 1,0,0 }) >= 0) if (x == 0) { world[chunk - to1D({ 1,0,0 })].canBeUpdated = true; }
					if (chunk - to1D({ 0,1,0 }) >= 0) if (y == 0) { world[chunk - to1D({ 0,1,0 })].canBeUpdated = true; }
					if (chunk - to1D({ 0,0,1 }) >= 0) if (z == 0) { world[chunk - to1D({ 0,0,1 })].canBeUpdated = true; }

					if (chunk + to1D({ 1,0,0 }) < world.size()) if (x == chunkSize - 1) { world[chunk + to1D({ 1,0,0 })].canBeUpdated = true; }
					if (chunk + to1D({ 0,1,0 }) < world.size()) if (y == chunkSize - 1) { world[chunk + to1D({ 0,1,0 })].canBeUpdated = true; }
					if (chunk + to1D({ 0,0,1 }) < world.size()) if (z == chunkSize - 1) { world[chunk + to1D({ 0,0,1 })].canBeUpdated = true; }
				}
			}
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block) {
			int16_t chunk = getChunkID(getChunkPos(pos));

			if (chunk < world.size() && chunk > 0) {
				glm::vec3  blockPos = getBlockPos(pos);
				int8_t x = blockPos.x;
				int8_t y = blockPos.y;
				int8_t z = blockPos.z;

				if (world[chunk].chunkData[x][y][z] != block) {

					world[chunk].chunkData[x][y][z] = block;
					world[chunk].canBeUpdated = true;

					int32_t neighborXpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 1,0,0 });
					int32_t neighborYpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,1,0 });
					int32_t neighborZpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,0,1 });

					int32_t neighborXneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 1,0,0 });
					int32_t neighborYneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,1,0 });
					int32_t neighborZneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,0,1 });

					/*if (blockData[block].emitLight) {
						gl::Light light;
						light.vector = glm::vec4(pos, 1.0f);
						light.ambient = glm::vec3(1.0f);
						light.diffuse = glm::vec3(1.0f);
						light.specular = glm::vec3(1.0f);
						light.constant = 1.0f;
						light.linear = 0.09f;
						light.quadratic = 0.032f;
						light.strenght = 1.f;
						light.Apply(worldShader, "lights[" + std::to_string(currentLight) + "]");
						currentLight++;
						if (currentLight >= 16) currentLight = 0;
					}*/

					if (neighborXneg >= 0) if (x == 0) { world[neighborXneg].canBeUpdated = true; }
					if (neighborYneg >= 0) if (y == 0) { world[neighborYneg].canBeUpdated = true; }
					if (neighborZneg >= 0) if (z == 0) { world[neighborZneg].canBeUpdated = true; }

					if (neighborXpos >= 0) if (x == chunkSize - 1) { world[neighborXpos].canBeUpdated = true; }
					if (neighborYpos >= 0) if (y == chunkSize - 1) { world[neighborYpos].canBeUpdated = true; }
					if (neighborZpos >= 0) if (z == chunkSize - 1) { world[neighborZpos].canBeUpdated = true; }
				}
			}
		}

		void UpdateMesh(const ChunkID& chunk) {

			if (chunk < world.size() && chunk > 0) {
				uint32_t offset = 0;
				uint32_t foffset = 0;
				world[chunk].IndexCount = 0;

				gl::Vertex worldMesh[MaxVertexCount];
				int32_t neighborXpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 1,0,0 });
				int32_t neighborYpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,1,0 });
				int32_t neighborZpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,0,1 });

				int32_t neighborXneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 1,0,0 });
				int32_t neighborYneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,1,0 });
				int32_t neighborZneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,0,1 });

				for (int8_t y = 0; y < chunkSize; y++)
					for (int8_t z = 0; z < chunkSize; z++)
						for (int8_t x = 0; x < chunkSize; x++)
						{
							BlockID block = world[chunk].chunkData[x][y][z];
							if (makeFace({ x,y,z }, chunk, ConnectionType::CONNECT_DEFAULT)) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}

								else if (neighborYpos >= 0) {
									BlockID checkBlock = world[neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborZpos >= 0) {
									BlockID checkBlock = world[neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborXpos >= 0) {
									BlockID checkBlock = world[neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborYneg >= 0) {
									BlockID checkBlock = world[neighborYneg].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborZneg >= 0) {
									BlockID checkBlock = world[neighborZneg].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborXneg >= 0) {
									BlockID checkBlock = world[neighborXneg].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::NO_CONNECT)) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}

								else if (neighborYpos >= 0) {
									BlockID checkBlock = world[neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborZpos >= 0) {
									BlockID checkBlock = world[neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborXpos >= 0) {
									BlockID checkBlock = world[neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}
								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else  if (neighborYneg >= 0) {
									BlockID checkBlock = world[neighborYneg].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborZneg >= 0) {
									BlockID checkBlock = world[neighborZneg].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborXneg >= 0) {
									BlockID checkBlock = world[neighborXneg].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									}
								}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::FLUID_CONNECT)) // Can make a fluid face
							{
								//Positive
								if (y + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 2, world[chunk].IndexCount, offset, worldMesh);
								}
								else if (neighborYpos >= 0) {
									BlockID checkBlock = world[neighborYpos].chunkData[x][0][z];
									if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 2, world[chunk].IndexCount, offset, worldMesh);
								}

								//if (x + 1 < chunkSize) {
								//	BlockID checkBlock = world[chunk].chunkData[x + 1][y][z];
								//	if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
								//		addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].fIndexCount, foffset, worldFluidMesh);
								//}
								//else if (neighborXpos >= 0) {
								//	BlockID checkBlock = world[neighborYpos].chunkData[0][y][z];
								//	if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
								//		addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].fIndexCount, foffset, worldFluidMesh);
								//}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::X_CONNECT)) // Can make a fluid face
							{
									addFace(X_FACE1, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									addFace(X_FACE2, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), 1, world[chunk].IndexCount, offset, worldMesh);
									//addFace(X_FACE3, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									//addFace(X_FACE4, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
						}
				worldIndexBuffer.Bind();

				world[chunk].chunkMeshArray.Bind();
				if (world[chunk].IndexCount > 0) world[chunk].chunkMeshBuffer.Update(0, sizeof(worldMesh), &worldMesh);
				
			}
		}			

		uint8_t textureSizeX = 1;
		uint8_t textureSizeY = 1;

		glm::vec2 TexCoords[4] = {
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f,         textureSizeY),
			glm::vec2(textureSizeX, textureSizeY),
			glm::vec2(textureSizeX, 0.0f),
		};

		void addFace(const Face& face, const glm::vec3& pos, const uint32_t& texture, const glm::vec3& Normal, const int8_t& type, uint32_t& IndexCount, uint32_t& offset, gl::Vertex* mesh) {
			if (IndexCount <= MaxFaceCount * 6) {		
				for (uint8_t i = 0; i < 4; i++) { mesh[i + offset] = gl::Vertex(face[i] + pos, { TexCoords[i], texture}, Normal, type); }
				IndexCount += 6;
				offset += 4;
			}
			else
				WC_ERROR("Memory overflow!");
		}		

		bool makeFace(const glm::vec3& pos, const ChunkID& chunkID, ConnectionType type) {
			if (pos.x >= chunkSize || pos.y >= chunkSize || pos.z >= chunkSize) return false;
			if (pos.x < 0 || pos.y < 0 || pos.z < 0) return false;
			if (chunkID >= world.size()) return false;
			if (chunkID < 0) return false;
			int8_t x = pos.x;
			int8_t y = pos.y;
			int8_t z = pos.z;
			BlockID block = world[chunkID].chunkData[x][y][z];
			if (block > 0 && blockData[block].blockConnectionType == type) return true;
			return false;
		}		

		glm::vec3 getBlockPos(const int& x, const int& y, const int& z)
		{
			return glm::floor(glm::vec3{ x % chunkSize, y % chunkSize, z %  chunkSize });
		}

		glm::vec3 getBlockPos(const glm::ivec3& pos)
		{
			return glm::floor(glm::vec3{ pos.x % chunkSize, pos.y % chunkSize, pos.z % chunkSize });
		}

		glm::vec3 getChunkPos(const int& x, const int& y, const int& z)
		{
			return glm::floor(glm::vec3{ x / chunkSize, y / chunkSize, z / chunkSize });
		}

		glm::vec3 getChunkPos(const glm::vec3& pos)
		{
			return glm::floor(glm::vec3{ pos.x / chunkSize, pos.y / chunkSize, pos.z / chunkSize });
		}

		BlockID getBlock(const glm::vec3& pos) {
			ChunkID chunk = getChunkID(getChunkPos(pos));
			glm::vec3 blocPos = getBlockPos(pos);
			int8_t x = blocPos.x;
			int8_t y = blocPos.y;
			int8_t z = blocPos.z;
			return world[chunk].chunkData[x][y][z];
		}		

		int32_t getChunkID(const glm::vec3& pos) {
			for (ChunkID i = 0; i < world.size(); i++) {
				if (world[i].chunkPos.x == pos.x &&
					world[i].chunkPos.y == pos.y &&
					world[i].chunkPos.z == pos.z) {
					return i;
				}
			}
			return -1;
		}

		gl::Shader worldShader;
		//gl::ShadowMap shadowMap;

		gl::IndexBuffer worldIndexBuffer;
		std::array<Chunk, chunkSize * chunkSize * chunkSize> world;
		Noise worldNoise;
		//Noise biomeNoise;

		int8_t water_level = 0;
		//int8_t snow_level = 0;
		int8_t currentLight = 0;
	};	
}
#endif