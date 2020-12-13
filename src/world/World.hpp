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

namespace wc {

	class Singleplayer : public NonCopyable {
	public:
		Player p;
		glm::vec4 screenColor = glm::vec4(1.0f);

		Singleplayer() {}

		void Create() {
			worldShader.Create("shaderpacks/default/chunkShader.glsl");
			fluidShader.Create("shaderpacks/default/fluidShader.glsl");

			LoadBlocks();
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

			p.InitPlayer({ chunkSize * chunkSize / 2 + chunkSize,chunkSize * 4,chunkSize * chunkSize / 2 });

			for (ChunkID i = 0; i < world.size(); i++) {
				world[i].chunkPos = to3D(i);
				//Configuring the vertex array
				world[i].chunkMeshArray.Create();
				world[i].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				world[i].chunkMeshArray.VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				world[i].chunkMeshArray.VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
			}
		}

		int GetChunkPos(const int& pos) {
			int i = pos;
			int junk = 0;
			if (i % chunkSize != 0) { junk = i % chunkSize; i = i - junk; return i / chunkSize; }
			else { return i / chunkSize; }
		}

		void Update(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {
			p.UpdatePlayer(windpos, windsize, CenterMouse, deltaTime);

			blockAtlas.Bind();
			worldIndexBuffer.Bind(); // Binding the index buffer

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("u_Projection", p.projection);

			// camera/view transformation
			worldShader.setMat4("u_View", p.GetView());
			frame++;
			if (frame >= world.size()) frame = 0;
			for (ChunkID i = 0; i < world.size(); i++) {

				uint8_t chunkHalf = chunkSize / 2;
				glm::vec3 currChunkPos = world[i].chunkPos;

				//if (currChunkPos.x <= GetChunkPos(p.Position.x) - chunkHalf) { world[i].chunkPos = { currChunkPos.x + 1, currChunkPos.y, currChunkPos.z }; }
				if (currChunkPos.x >= GetChunkPos(p.Position.x) + chunkHalf) ResetChunk(i, glm::vec3(GetChunkPos(p.Position.x) - chunkHalf, currChunkPos.y, currChunkPos.z));
				

				if (world[i].IndexCount > 0) {
					world[i].chunkMeshArray.Bind();
					glm::vec3 pos = glm::vec3(1.0f) + world[i].chunkPos * glm::vec3(chunkSize);
					worldShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), pos)); // calculate the model matrix for each object and pass it to shader before drawing
					glDrawElements(GL_TRIANGLES, world[i].IndexCount, GL_UNSIGNED_INT, nullptr); // Drawing the cubes
				}

				// Updating the chunk`s mesh
				if (world[i].canBeUpdated) {
					UpdateMesh(i);
					world[i].canBeUpdated = false;
				}
			}
		}

		void OnEvent(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
			//if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON || wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) {
			//	glm::vec3 m_rayLastPos = glm::vec3(0.0f);
			//	for (Ray ray(p.camera.Position); ray.getLength() < 4; ray.Step(p.camera.Yaw, p.camera.Pitch)) {
			//		int chunk = to1D({ GetChunkPos(ray.getEnd().x) ,GetChunkPos(ray.getEnd().y) ,GetChunkPos(ray.getEnd().z) });
			//		int8_t x = ray.getEnd().x;
			//		int8_t y = ray.getEnd().y;
			//		int8_t z = ray.getEnd().z;
			//		if(world[chunk].chunkData[x][y][z] > 0)
			//		if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) {
			//			setBlock(p.Position, 6);
			//		}
			//		if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) {
			//			setBlock(ray.getEnd(), 6);
			//		}
			//		m_rayLastPos = ray.getEnd();
			//	}
			//}
		}

	private:

		void ResetChunk(const ChunkID& chunk, const glm::vec3& newChunkPos) {
			world[chunk].chunkPos = newChunkPos;
			//world[chunk].generated = false;
			//world[chunk].canBeUpdated = true;
		}

		//Chunk managing

		void GenerateChunkTerrain(const ChunkID& chunk) {
			if (!world[chunk].generated) {
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap = worldNoise.getNoiseFor(glm::vec2(x, z), glm::vec2(world[chunk].chunkPos.x, world[chunk].chunkPos.z), chunkSize);
						//int heightMap = 34;
						for (uint8_t y = 0; y < chunkSize; y++) {
							int32_t pos = (int)world[chunk].chunkPos.y * chunkSize + y;
							if (pos == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
							if (pos == heightMap && pos <= water_level + 1 - rand() % 3) { setBlock(glm::vec3(x, y, z), 4, chunk); }
							if (pos > heightMap && pos < water_level) { setBlock(glm::vec3(x, y, z), 5, chunk); }
							if (pos < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
							if (pos < heightMap - rand() % 3) { setBlock(glm::vec3(x, y, z), 3, chunk); }
							if (pos == heightMap && rand() % 100 > 98) setBlock(glm::vec3(x, y, z), 7, chunk);
							if (pos == heightMap && pos > snow_level) { setBlock(glm::vec3(x, y + 1, z), 8, chunk); }
							if (pos == heightMap && pos > snow_level - 1 + rand() % 5) { setBlock(glm::vec3(x, y + 1, z), 8, chunk); }
						}
					}
				world[chunk].generated = true;
			}
		}

		void setBlock(const glm::vec3& pos, const BlockID& block, const ChunkID& chunk) {
			if (chunk >= world.size()) return;
			if (chunk < 0) return;
			int8_t x = pos.x;
			int8_t y = pos.y;
			int8_t z = pos.z;
			world[chunk].chunkData[x][y][z] = block;
			world[chunk].canBeUpdated = true;

			if (chunk - to1D({ 1,0,0 }) >= 0) if (x == 0) { world[chunk - to1D({ 1,0,0 })].canBeUpdated = true; }
			if (chunk - to1D({ 0,1,0 }) >= 0) if (y == 0) { world[chunk - to1D({ 0,1,0 })].canBeUpdated = true; }
			if (chunk - to1D({ 0,0,1 }) >= 0) if (z == 0) { world[chunk - to1D({ 0,0,1 })].canBeUpdated = true; }

			if (chunk + to1D({ 1,0,0 }) < world.size()) if (x == chunkSize - 1) { world[chunk + to1D({ 1,0,0 })].canBeUpdated = true; }
			if (chunk + to1D({ 0,1,0 }) < world.size()) if (y == chunkSize - 1) { world[chunk + to1D({ 0,1,0 })].canBeUpdated = true; }
			if (chunk + to1D({ 0,0,1 }) < world.size()) if (z == chunkSize - 1) { world[chunk + to1D({ 0,0,1 })].canBeUpdated = true; }
		}

		void setBlock(const glm::vec3& pos, const BlockID& block) {
			int chunk = to1D({ GetChunkPos(pos.x) ,GetChunkPos(pos.y) ,GetChunkPos(pos.z) });
			if (chunk >= world.size()) return;
			if (chunk < 0) return;
			int8_t x = pos.x; x = x % chunkSize;
			int8_t y = pos.y; y = y % chunkSize;
			int8_t z = pos.z; z = z % chunkSize;

			if (x >= chunkSize) return;
			if (y >= chunkSize) return;
			if (z >= chunkSize) return;

			if (x < 0) return;
			if (y < 0) return;
			if (z < 0) return;
			world[chunk].chunkData[x][y][z] = block;
			world[chunk].canBeUpdated = true;

			if (chunk - to1D({ 1,0,0 }) >= 0) if (x == 0) { world[chunk - to1D({ 1,0,0 })].canBeUpdated = true; }
			if (chunk - to1D({ 0,1,0 }) >= 0) if (y == 0) { world[chunk - to1D({ 0,1,0 })].canBeUpdated = true; }
			if (chunk - to1D({ 0,0,1 }) >= 0) if (z == 0) { world[chunk - to1D({ 0,0,1 })].canBeUpdated = true; }

			if (chunk + to1D({ 1,0,0 }) < world.size()) if (x == chunkSize - 1) { world[chunk + to1D({ 1,0,0 })].canBeUpdated = true; }
			if (chunk + to1D({ 0,1,0 }) < world.size()) if (y == chunkSize - 1) { world[chunk + to1D({ 0,1,0 })].canBeUpdated = true; }
			if (chunk + to1D({ 0,0,1 }) < world.size()) if (z == chunkSize - 1) { world[chunk + to1D({ 0,0,1 })].canBeUpdated = true; }
		}

		int GetBlock(const glm::vec3& pos) {
			int chunk = to1D({ GetChunkPos(pos.x) ,GetChunkPos(pos.y) ,GetChunkPos(pos.z) });
			//if (chunk >= world.size()) return;
			//if (chunk < 0) return;
			int8_t x = pos.x; x = x % chunkSize;
			int8_t y = pos.y; y = y % chunkSize;
			int8_t z = pos.z; z = z % chunkSize;
			return world[chunk].chunkData[x][y][z];
		}

		glm::vec3 toBlockPos(const glm::vec3& pos) {
			int8_t x = pos.x; x = x % chunkSize;
			int8_t y = pos.y; y = y % chunkSize;
			int8_t z = pos.z; z = z % chunkSize;

			return { x,y,z };
		}

		void UpdateMesh(const ChunkID& chunk) {
			if (chunk >= world.size()) return;
			if (chunk < 0) return;

			if (world[chunk].generated == false) GenerateChunkTerrain(chunk);

			if (chunk + to1D({ 0,1,0 }) < world.size()) if (world[chunk + to1D({ 0,1,0 })].generated == false) GenerateChunkTerrain(chunk + to1D({ 0,1,0 }));
			if (chunk + to1D({ 0,0,1 }) < world.size()) if (world[chunk + to1D({ 0,0,1 })].generated == false) GenerateChunkTerrain(chunk + to1D({ 0,0,1 }));
			if (chunk + to1D({ 1,0,0 }) < world.size()) if (world[chunk + to1D({ 1,0,0 })].generated == false) GenerateChunkTerrain(chunk + to1D({ 1,0,0 }));

			uint32_t offset = 0;
			world[chunk].IndexCount = 0;

			gl::Vertex worldMesh[MaxVertexCount];
			gl::Vertex worldFluidMesh[MaxVertexCount];

			for (int8_t y = 0; y < chunkSize; y++)
				for (int8_t z = 0; z < chunkSize; z++)
					for (int8_t x = 0; x < chunkSize; x++)
					{
						if (makeFace({ x,y,z }, chunk, ConnectionType::CONNECT_DEFAULT)) // Can make block face
						{
							int block = world[chunk].chunkData[x][y][z];
							//Positive
							if (y + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x][y + 1][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}

							else if (chunk + to1D({ 0,1,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (z + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x][y][z + 1];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 0,0,1 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (x + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x + 1][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 1,0,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							//Negative	   																										 					  															    						 
							if (y - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x][y - 1][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else  if (chunk - to1D({ 0,1,0 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize - 1][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (z - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x][y][z - 1];
								if (world[chunk].chunkData[x][y][z - 1] == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk - to1D({ 0,0,1 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize - 1];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (x - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x - 1][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk - to1D({ 1,0,0 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize - 1][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}
						}

						if (makeFace({ x,y,z }, chunk, ConnectionType::NO_CONNECT)) // Can make block face
						{
							int block = world[chunk].chunkData[x][y][z];
							//Positive
							if (y + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x][y + 1][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}

							else if (chunk + to1D({ 0,1,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (z + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x][y][z + 1];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 0,0,1 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], glm::vec3(0.0f, 0.0f, 1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (x + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x + 1][y][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 1,0,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							//Negative	   																										 					  															    						 
							if (y - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x][y - 1][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else  if (chunk - to1D({ 0,1,0 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize - 1][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], glm::vec3(0.0f, -1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (z - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x][y][z - 1];
								if (world[chunk].chunkData[x][y][z - 1] == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk - to1D({ 0,0,1 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize - 1];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], glm::vec3(0.0f, 0.0f, -1.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}

							if (x - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x - 1][y][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk - to1D({ 1,0,0 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize - 1][y][z];
								if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], glm::vec3(-1.0f, 0.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
								}
							}
						}

						if (makeFace({ x,y,z }, chunk, ConnectionType::FLUID_CONNECT)) // Can make a fluid face
						{
							int block = world[chunk].chunkData[x][y][z];
							//Positive
							if (y + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x][y + 1][z];
								if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 0,1,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
								if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
						}
					}
			worldIndexBuffer.Bind();
			if (world[chunk].IndexCount > 0) {
				world[chunk].chunkMeshArray.Bind();
				world[chunk].chunkMeshBuffer.Update(0, sizeof(worldMesh), &worldMesh);
			}
		}
			

		void addFace(const Face& face, const glm::vec3& pos, const glm::vec2& Coords, const glm::vec3& Normal, uint32_t& IndexCount, uint32_t& offset, gl::Vertex* mesh) {
			if (IndexCount >= MaxFaceCount * 6) {
				WC_ERROR("Memory overflow!");
				return;
			}
		
			for (uint8_t i = 0; i < 4; i++) { mesh[i + offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteIndexCoords(Coords, { 32,32 })[i], Normal); }
			IndexCount += 6;
			offset += 4;
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
		} // @TODO

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

		int8_t water_level = 32;
		int8_t snow_level = 64;
		int frame;

		gl::Shader worldShader;
		gl::Shader fluidShader;

		gl::Texture blockAtlas;
		gl::IndexBuffer worldIndexBuffer;
		std::unordered_map<int, Block> blockData;
		std::array<Chunk, chunkSize * chunkSize * chunkSize> world;
		Noise worldNoise;
		//wc::ViewFrustum frustum;
	};
}
#endif