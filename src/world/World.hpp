// Game idea
// Space ship game where you go around planets, gather resources then go and fight people an invade their spaceships, until you invde all the galaxy
// Type: strategy, fps

#ifndef WORLD_HPP
#define WORLD_HPP

#include "Chunk.hpp"
#include "Block.hpp"
#include "../entityes/Player.hpp"
#include <Utils/Time.hpp>
#include <Maths/Frustum.hpp>
#include <Maths/Noise.hpp>
#include <map>

namespace wc {

	class Singleplayer : public NonCopyable {
	public:
		Player p;

		Singleplayer() {}

		void Create() {
			worldShader.Create("shaderpacks/default/chunkShader.glsl");
			fluidShader.Create("shaderpacks/default/fluidShader.glsl");
			sol::state noiseState;
			noiseState.script_file("scripts/worldGen.lua");
			worldNoise.lacunarity = noiseState["lacunarity"];
			worldNoise.multiplier = noiseState["multiplier"];
			worldNoise.octaves = noiseState["octaves"];
			worldNoise.persistance = noiseState["persistance"];
			worldNoise.scale = noiseState["scale"];
			worldNoise.seed = noiseState["seed"];
			LoadBlocks();			

			p.InitPlayer({ chunkSize * chunkSize / 2 + chunkSize,chunkSize * 4,chunkSize * chunkSize / 2 });

			for (ChunkID i = 0; i < world.size(); i++) {
				world[i].chunkPos = to3D(i);
				//Configuring the vertex array
				world[i].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				world[i].chunkMeshArray.Create();
				world[i].chunkMeshArray.VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				world[i].chunkMeshArray.VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute

				//Configuring the vertex array
				world[i].chunkFluidBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				world[i].chunkFluidArray.Create();
				world[i].chunkFluidArray.VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				world[i].chunkFluidArray.VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
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
		}

		void Update(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {
			p.UpdatePlayer(windpos, windsize, CenterMouse, deltaTime);

			blockAtlas.Bind();			

			// activate shader
			fluidShader.use();
			
			// pass projection matrix to shader (note that in this case it could change every frame)
			fluidShader.setMat4("u_Projection", p.projection);
			
			// camera/view transformation
			fluidShader.setMat4("u_View", p.GetView());

			if (waveTime < 1.3) waveTime += 0.0001; 
			else waveTime = 0.0;
			fluidShader.setFloat("waveTime", waveTime);

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.camera.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("u_Projection", p.projection);

			// camera/view transformation
			worldShader.setMat4("u_View", p.GetView());

			for (ChunkID i = 0; i < world.size(); i++) {

				uint8_t chunkHalf = chunkSize / 2;
				glm::vec3 currChunkPos = world[i].chunkPos;
				glm::vec3 pos = world[i].chunkPos * glm::vec3(chunkSize);

				if (world[i].IndexCount > 0) {
					worldShader.use();
					world[i].chunkMeshArray.Bind();
					worldShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), pos)); // calculate the model matrix for each object and pass it to shader before drawing
					glDrawElements(GL_TRIANGLES, world[i].IndexCount, GL_UNSIGNED_INT, nullptr); // Drawing the cubes
				}

				if (world[i].fIndexCount > 0) {
					world[i].chunkFluidArray.Bind();
					fluidShader.use();
					fluidShader.setVec3("chunkPos", pos);
					fluidShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), (glm::vec3)pos)); // calculate the model matrix for each object and pass it to shader before drawing
					glDisable(GL_CULL_FACE);
					glDrawElements(GL_TRIANGLES, world[i].fIndexCount, GL_UNSIGNED_INT, nullptr); // Drawing the cubes
					glEnable(GL_CULL_FACE);
				}				

				if (currChunkPos.x < glm::floor(p.Position.x / chunkSize) - chunkHalf) ResetChunk(i, glm::vec3(glm::floor(p.Position.x / chunkSize) + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > glm::floor(p.Position.x / chunkSize) + chunkHalf) ResetChunk(i, glm::vec3(glm::floor(p.Position.x / chunkSize) - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				if (currChunkPos.y < glm::floor(p.Position.y / chunkSize) - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, glm::floor(p.Position.y / chunkSize) - chunkHalf + 1, currChunkPos.z));
				if (currChunkPos.y > glm::floor(p.Position.y / chunkSize) + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, glm::floor(p.Position.y / chunkSize) + chunkHalf - 1, currChunkPos.z));

				if (currChunkPos.z > glm::floor(p.Position.z / chunkSize) + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, glm::floor(p.Position.z / chunkSize) - chunkHalf + 1));
				if (currChunkPos.z < glm::floor(p.Position.z / chunkSize) - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, glm::floor(p.Position.z / chunkSize) + chunkHalf - 1));

				// Updating the chunk`s mesh
				if (!world[i].generated) GenerateChunkTerrain(i);
				if (world[i].canBeUpdated) {

					UpdateMesh(i);
					world[i].canBeUpdated = false;
				}
			}
		}

		void OnInput(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
			if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON || wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) {
				glm::vec3 m_rayLastPos = glm::vec3(0.0f);
				for (Ray ray(p.Position); ray.getLength() < 4; ray.Step(p.camera.Yaw, p.camera.Pitch)) {				

					if (getBlock(ray.getEnd()) > 0)	if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON) { setBlock(ray.getEnd(), 0); break; }
						else if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) { setBlock(m_rayLastPos, p.ItemHolding); break; }
					
					m_rayLastPos = ray.getEnd();
				}
			}
		}

		float waveTime = 1.3f;
	private:

		//Chunk managing

		void ResetChunk(const ChunkID& chunk, const glm::vec3& newChunkPos) {
			world[chunk].chunkPos = newChunkPos;
			world[chunk].generated = false;
			UpdateMesh(chunk);
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {
			if (!world[chunk].generated) {

				int8_t water_level = 17;
				int8_t snow_level = 1;
				memset(&world[chunk].chunkData, 0, sizeof(world[chunk].chunkData));
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) {
						int voxelX = x + world[chunk].chunkPos.x * chunkSize;
						int voxelZ = z + world[chunk].chunkPos.z * chunkSize;
						int heightMap = worldNoise.getNoiseFor(voxelX, voxelZ);
						for (uint8_t y = 0; y < chunkSize; y++) {
							int32_t pos = glm::floor(world[chunk].chunkPos.y * chunkSize + y);
							if (pos == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
							if (pos == heightMap && pos <= water_level + 1 - rand() % 3) { setBlock(glm::vec3(x, y, z), 4, chunk); }
							if (pos > heightMap && pos < water_level) { setBlock(glm::vec3(x, y, z), 5, chunk); }
							if (pos < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
							if (pos < heightMap - rand() % 3) { setBlock(glm::vec3(x, y, z), 3, chunk); }
							if (pos == heightMap && rand() % 100 > 98 && pos > water_level) setBlock(glm::vec3(x, y, z), 7, chunk);
							//if (pos == heightMap && pos >= snow_level) { setBlock(glm::vec3(x, y + 1, z), 8, chunk); }
							//if (pos == heightMap && pos > snow_level - 1 + rand() % 5) { setBlock(glm::vec3(x, y + 1, z), 8, chunk); }
						}
					}
				world[chunk].generated = true;
			}
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block, const ChunkID& chunk) {
			if (chunk < world.size() && chunk > 0) {
				int8_t x = pos.x;
				int8_t y = pos.y;
				int8_t z = pos.z;
				if (x >= chunkSize) x = getBlockPos(pos).x;
				if (y >= chunkSize) y = getBlockPos(pos).y;
				if (z >= chunkSize) z = getBlockPos(pos).z;
				if (world[chunk].chunkData[x][y][z] != block) {
					world[chunk].chunkData[x][y][z] = block;
					world[chunk].canBeUpdated = true;

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

			if (chunk < 0)  return; 
			int8_t x = getBlockPos(pos).x;
			int8_t y = getBlockPos(pos).y;
			int8_t z = getBlockPos(pos).z;

			world[chunk].chunkData[x][y][z] = block;
			world[chunk].canBeUpdated = true;

			if (chunk - to1D({ 1,0,0 }) >= 0) if (x == 0) { world[chunk - to1D({ 1,0,0 })].canBeUpdated = true; }
			if (chunk - to1D({ 0,1,0 }) >= 0) if (y == 0) { world[chunk - to1D({ 0,1,0 })].canBeUpdated = true; }
			if (chunk - to1D({ 0,0,1 }) >= 0) if (z == 0) { world[chunk - to1D({ 0,0,1 })].canBeUpdated = true; }

			if (chunk + to1D({ 1,0,0 }) < world.size()) if (x == chunkSize - 1) { world[chunk + to1D({ 1,0,0 })].canBeUpdated = true; }
			if (chunk + to1D({ 0,1,0 }) < world.size()) if (y == chunkSize - 1) { world[chunk + to1D({ 0,1,0 })].canBeUpdated = true; }
			if (chunk + to1D({ 0,0,1 }) < world.size()) if (z == chunkSize - 1) { world[chunk + to1D({ 0,0,1 })].canBeUpdated = true; }
		}

		void UpdateMesh(const ChunkID& chunkID) {
			ChunkID chunk = chunkID;
			if (chunk < world.size() && chunk > 0) {
				uint32_t offset = 0;
				uint32_t foffset = 0;
				world[chunk].IndexCount = 0;
				world[chunk].fIndexCount = 0;

				gl::Vertex worldMesh[MaxVertexCount];
				gl::Vertex worldFluidMesh[MaxVertexCount];

				for (int8_t y = 0; y < chunkSize; y++)
					for (int8_t z = 0; z < chunkSize; z++)
						for (int8_t x = 0; x < chunkSize; x++)
						{
							if (makeFace({ x,y,z }, chunk, ConnectionType::CONNECT_DEFAULT)) // Can make block face
							{
								BlockID block = world[chunk].chunkData[x][y][z];
								//Positive
								if (y + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}

								else if (chunk + to1D({ 0,1,0 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk + to1D({ 0,0,1 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk + to1D({ 1,0,0 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk - to1D({ 0,1,0 }) >= 0) {
									BlockID checkBlock = world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk - to1D({ 0,0,1 }) >= 0) {
									BlockID checkBlock = world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk - to1D({ 1,0,0 }) >= 0) {
									BlockID checkBlock = world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::NO_CONNECT)) // Can make block face
							{
								BlockID block = world[chunk].chunkData[x][y][z];
								//Positive
								if (y + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}

								else if (chunk + to1D({ 0,1,0 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk + to1D({ 0,0,1 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk + to1D({ 1,0,0 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else  if (chunk - to1D({ 0,1,0 }) >= 0) {
									BlockID checkBlock = world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (z - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk - to1D({ 0,0,1 }) >= 0) {
									BlockID checkBlock = world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}

								if (x - 1 >= 0) {
									BlockID checkBlock = world[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
								else if (chunk - to1D({ 1,0,0 }) >= 0) {
									BlockID checkBlock = world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									}
								}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::FLUID_CONNECT)) // Can make a fluid face
							{
								BlockID block = world[chunk].chunkData[x][y][z];
								//Positive
								if (y + 1 < chunkSize) {
									BlockID checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].fIndexCount, foffset, worldFluidMesh);
								}
								else if (chunk + to1D({ 0,1,0 }) < world.size()) {
									BlockID checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
									if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].fIndexCount, foffset, worldFluidMesh);
								}
							}
						}
				worldIndexBuffer.Bind();

				world[chunk].chunkMeshArray.Bind();
				if (world[chunk].IndexCount > 0) world[chunk].chunkMeshBuffer.Update(0, sizeof(worldMesh), &worldMesh);

				worldIndexBuffer.Bind();

				world[chunk].chunkFluidArray.Bind();
				if (world[chunk].fIndexCount > 0) world[chunk].chunkFluidBuffer.Update(0, sizeof(worldFluidMesh), &worldFluidMesh);
				
			}
		}			

		void addFace(const Face& face, const glm::vec3& pos, const glm::vec2& Coords, const glm::vec3& Normal, uint32_t& IndexCount, uint32_t& offset, gl::Vertex* mesh) {
			if (IndexCount <= MaxFaceCount * 6) {		
				for (uint8_t i = 0; i < 4; i++) { mesh[i + offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteIndexCoords(Coords, { 32,32 })[i], Normal); }
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
			int16_t x = pos.x;
			int16_t y = pos.y;
			int16_t z = pos.z;
			BlockID block = world[chunkID].chunkData[x][y][z];
			if (block > 0 && blockData[block].blockConnectionType == type) return true;
			return false;
		} 

		void LoadBlocks() {
			blockData.reserve(7);
			blockAtlas.load("assets/textures/block/blockAtlas.png");

			AddBlock("scripts/grassblock.lua", {
				glm::vec2(0, 1),
				glm::vec2(1, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0) });

			AddBlock("scripts/dirt.lua", {
				glm::vec2(1, 0),
				glm::vec2(1, 0),
				glm::vec2(1, 0),
				glm::vec2(1, 0),
				glm::vec2(1, 0),
				glm::vec2(1, 0) });

			AddBlock("scripts/stone.lua", {
				glm::vec2(1, 1),
				glm::vec2(1, 1),
				glm::vec2(1, 1),
				glm::vec2(1, 1),
				glm::vec2(1, 1),
				glm::vec2(1, 1) });

			AddBlock("scripts/sand.lua", {
				glm::vec2(2, 1),
				glm::vec2(2, 1),
				glm::vec2(2, 1),
				glm::vec2(2, 1),
				glm::vec2(2, 1),
				glm::vec2(2, 1) });

			AddBlock("scripts/water.lua", {
				glm::vec2(5, 0),
				glm::vec2(5, 0),
				glm::vec2(5, 0),
				glm::vec2(5, 0),
				glm::vec2(5, 0),
				glm::vec2(5, 0) });

			AddBlock("scripts/glass.lua", {
				glm::vec2(0, 2),
				glm::vec2(0, 2),
				glm::vec2(0, 2),
				glm::vec2(0, 2),
				glm::vec2(0, 2),
				glm::vec2(0, 2) });

			AddBlock("scripts/wood.lua", {
				glm::vec2(3, 1),
				glm::vec2(3, 1),
				glm::vec2(3, 0),
				glm::vec2(3, 0),
				glm::vec2(3, 0),
				glm::vec2(3, 0) });

			AddBlock("scripts/snow.lua", {
				glm::vec2(2, 2),
				glm::vec2(2, 2),
				glm::vec2(2, 2),
				glm::vec2(2, 2),
				glm::vec2(2, 2),
				glm::vec2(2, 2) });
		}

		glm::ivec3 getBlockPos(int x, int y, int z)
		{
			return { glm::floor(x % chunkSize), glm::floor(y % chunkSize), glm::floor(z %  chunkSize) };
		}

		glm::ivec3 getBlockPos(glm::ivec3 pos)
		{
			return { glm::floor(pos.x % chunkSize), glm::floor(pos.y % chunkSize), glm::floor(pos.z % chunkSize) };
		}

		glm::ivec3 getChunkPos(int x, int y, int z)
		{
			return { glm::floor(x / chunkSize), glm::floor(y / chunkSize), glm::floor(z / chunkSize) };
		}

		glm::ivec3 getChunkPos(glm::ivec3 pos)
		{
			return { glm::floor(pos.x / chunkSize), glm::floor(pos.y / chunkSize), glm::floor(pos.z / chunkSize) };
		}

		BlockID getBlock(const glm::vec3& pos) {
			ChunkID chunk = getChunkID(getChunkPos(pos));
			int8_t x = getBlockPos(pos).x;
			int8_t y = getBlockPos(pos).y;
			int8_t z = getBlockPos(pos).z;
			return world[chunk].chunkData[x][y][z];
		}

		void AddBlock(const char* script, const std::array<glm::vec2, 6>& texCoords) {
			Block block;
			block.Create(script);
			block.TexCoords[(int)BlockTexture::TOP] = texCoords[(int)BlockTexture::TOP];
			block.TexCoords[(int)BlockTexture::BOTTOM] = texCoords[(int)BlockTexture::BOTTOM];
			block.TexCoords[(int)BlockTexture::LEFT] = texCoords[(int)BlockTexture::LEFT];
			block.TexCoords[(int)BlockTexture::RIGHT] = texCoords[(int)BlockTexture::RIGHT];
			block.TexCoords[(int)BlockTexture::FRONT] = texCoords[(int)BlockTexture::FRONT];
			block.TexCoords[(int)BlockTexture::BACK] = texCoords[(int)BlockTexture::BACK];
			blockData[block.id] = block;
		}

		int16_t getChunkID(const glm::vec3& pos) {
			for (ChunkID i = 0; i < world.size(); i++)
				if (world[i].chunkPos == pos) return i;
			WC_ERROR("Cant find chunk at location: X:{0} Y:{1}, Z:{2}", pos.x, pos.y, pos.z);
			return -1;
		}

		gl::Shader worldShader;
		gl::Shader fluidShader;

		gl::Texture blockAtlas;
		gl::IndexBuffer worldIndexBuffer;
		std::unordered_map<int, Block> blockData;
		std::array<Chunk, chunkSize * chunkSize * chunkSize> world;
		Noise worldNoise;
	};
}
#endif