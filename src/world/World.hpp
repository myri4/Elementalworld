#pragma once

#include "Chunk.hpp"
#include "Block.hpp"
#include "../entityes/Player.hpp"
#include <Utils/Time.hpp>
#include <gl/Particle.hpp>

namespace wc {

	struct Noise {
		int octaves = 5;
		float smoothnes = 90.0f;
		int seed = 10;
		float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition)
		{
			int voxelX = pos.x + chunkPosition.x * chunkSize;
			int voxelZ = pos.y + chunkPosition.y * chunkSize;

			float value = 0.0f;
			for (int i = 0; i < octaves; i++) {
				value = glm::simplex(glm::vec3(voxelX / smoothnes + seed, voxelZ / smoothnes + seed, seed));
			}
			value = (value + 1) / 2;
			value *= 32 + 32;
			return value;
		}
	};

	//float rounded(const glm::vec2& coord)
	//{
	//	auto bump = [](float t) { return glm::max(0.0f, 1.0f - glm::pow(t, 6.0f)); };
	//	float b = bump(coord.x) * bump(coord.y);
	//	return b * 0.9f;
	//}	


	class World : public NonCopyable {
	public:
		Player p;

		gl::VertexBuffer scrQuad;
		gl::Shader screenShader;
		gl::FrameBuffer screen;

		World() {}
		~World() {}

		void Create() {
			worldShader.Create("shaderpacks/default/chunkShader.glsl");
			worldShader.use();
			worldShader.setInt("u_Texture", 0);

			fluidShader.Create("shaderpacks/default/fluidShader.glsl");
			fluidShader.use();
			fluidShader.setInt("u_Texture", 0);


			screenShader.Create("shaderpacks/default/screenShader.glsl");
			screenShader.use();
			screenShader.setInt("screenTexture", 0);

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f,  1.0f,  1.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,
			};

			scrQuad.Create(quadVertices, sizeof(quadVertices));

			LoadBlocks();
			LoadEnivoirment();
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

			p.InitPlayer(glm::vec3(chunkSize * chunkSize / 2, chunkSize * 4, chunkSize * chunkSize / 2));
			for (int i = 0; i < world.size(); i++) {
				world[i].Create(i);
			
				GenerateChunkTerrain(i);
				UpdateWorldMesh(i);
			}

			screen.Create(1280, 720);
			/*gl::Material material;
			
			material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
			material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
			material.shininess = 0.0f;
			material.Apply(worldShader, "material");*/
		}

		void Update(const sf::RenderWindow& window, const bool& CenterMouse, const float& deltaTime) {
			//screen.Bind();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			p.UpdatePlayer({ window.getPosition().x, window.getPosition().y }, CenterMouse, deltaTime);

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("u_Projection", p.projection);
			blockAtlas.Bind();

			// camera/view transformation
			worldShader.setMat4("u_View", p.GetView());			

			if (worldIndexBuffer.GetEBO()) {
				worldIndexBuffer.Bind(); // Binding the index buffer
				for (uint16_t i = 0; i < world.size(); i++) {
					if (world[i].chunkMeshBuffer.GetVAO()) {// Checking if the index and vertex buffer are actually created, if not then return
						worldShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), to3D(world[i].chunkPosition) * glm::vec3(chunkSize))); // calculate the model matrix for each object and pass it to shader before drawing
						//if (world[i].IndexCount <= 0) return;
						world[i].chunkMeshBuffer.BindVAO(); // Binding the vertex buffer
						glDrawElements(GL_TRIANGLES, world[i].IndexCount, GL_UNSIGNED_INT, nullptr); // Drawing the cubes
					}
				}
			}

			//fluidShader.use();
			//// pass projection matrix to shader (note that in this case it could change every frame)
			//fluidShader.setMat4("u_Projection", p.projection);
			//
			//// camera/view transformation
			//fluidShader.setMat4("u_View", p.GetView());
			//
			//for (uint16_t i = 0; i < world.size(); i++) {
			//	if (world[i].chunkFluidMeshBuffer.GetVAO() == 0 || worldIndexBuffer.GetEBO() == 0) return; // Checking if the index and vertex buffer are actually created, if not then return
			//	fluidShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), to3D(world[i].chunkPosition) * glm::vec3(chunkSize))); // calculate the model matrix for each object and pass it to shader before drawing
			////	if (world[i].IndexCount <= 0) return;
			//	world[i].chunkFluidMeshBuffer.Bind(); // Binding the vertex buffer
			//	glDisable(GL_CULL_FACE);
			//	glDrawElements(GL_TRIANGLES, world[i].fIndexCount, GL_UNSIGNED_INT, nullptr); // Drawing fluids
			//	glEnable(GL_CULL_FACE);
			//}
			
			// Checking if the fog is enabled, if not draw the skybox
			if (!Fog) skybox.Draw(glm::mat4(glm::mat3(p.GetView())), p.projection);			

			// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
			//screen.unbind();
			glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
			// clear all relevant buffers
			//glClear(GL_COLOR_BUFFER_BIT);
			//
			//screenShader.use();
			scrQuad.BindVAO();
			//screen.BindTexture();
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void OnEvent(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
			
			if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON || wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) {
				glm::vec3 m_rayLastPoint = glm::vec3(0.0f);
				Ray ray(p.camera.Position);
				for (; ray.getLength() < 8 * 6; ray.Step(p.camera.Yaw, p.camera.Pitch)) {
					if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON) {
						int chunk = to1D({ GetChunkPos(ray.getEnd().x) ,GetChunkPos(ray.getEnd().y) ,GetChunkPos(ray.getEnd().z) });
						setBlock(ray.getEnd(), 0); 
						UpdateWorldMesh(chunk);
						int x = ray.getEnd().x; x = x % chunkSize;
						int y = ray.getEnd().y; y = y % chunkSize;
						int z = ray.getEnd().z; z = z % chunkSize;
						if (x == 0) { UpdateWorldMesh(chunk - to1D({ 1,0,0 })); }
						if (y == 0) { UpdateWorldMesh(chunk - to1D({ 0,1,0 })); }
						if (z == 0) { UpdateWorldMesh(chunk - to1D({ 0,0,1 })); }

						if (x == chunkSize - 1) { UpdateWorldMesh(chunk + to1D({ 1,0,0 })); }
						if (y == chunkSize - 1) { UpdateWorldMesh(chunk + to1D({ 0,1,0 })); }
						if (z == chunkSize - 1) { UpdateWorldMesh(chunk + to1D({ 0,0,1 })); }
						
						break;
					}
					if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) { 
						int chunk = to1D({ GetChunkPos(m_rayLastPoint.x) ,GetChunkPos(m_rayLastPoint.y) ,GetChunkPos(m_rayLastPoint.z)});
						setBlock(m_rayLastPoint, 1);

						UpdateWorldMesh(chunk);
						int x = m_rayLastPoint.x; x = x % chunkSize;
						int y = m_rayLastPoint.y; y = y % chunkSize;
						int z = m_rayLastPoint.z; z = z % chunkSize;
						if (x == 0) { UpdateWorldMesh(chunk - to1D({ 1,0,0 })); }
						if (y == 0) { UpdateWorldMesh(chunk - to1D({ 0,1,0 })); }
						if (z == 0) { UpdateWorldMesh(chunk - to1D({ 0,0,1 })); }

						if (x == chunkSize - 1) { UpdateWorldMesh(chunk + to1D({ 1,0,0 })); }
						if (y == chunkSize - 1) { UpdateWorldMesh(chunk + to1D({ 0,1,0 })); }
						if (z == chunkSize - 1) { UpdateWorldMesh(chunk + to1D({ 0,0,1 })); }
						break;
					}
					m_rayLastPoint = ray.getEnd();
				}
			}
		}

	private:
		
		void GenerateChunkTerrain(const int& chunk) {
			Noise worldNoise;
			for (uint8_t z = 0; z < chunkSize; z++)
				for (uint8_t x = 0; x < chunkSize; x++) {
					int heightMap = worldNoise.getNoiseFor(glm::vec2(x, z), glm::vec2(to3D(world[chunk].chunkPosition).x - 2, to3D(world[chunk].chunkPosition).z));
					for (int y = 0; y < chunkSize; y++) {
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y == heightMap && (int)to3D(world[chunk].chunkPosition).y * chunkSize + y <= water_level - rand() % 3) { setBlock(glm::vec3(x, y, z), 4, chunk); }
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y > heightMap && (int)to3D(world[chunk].chunkPosition).y * chunkSize + y < water_level) { setBlock(glm::vec3(x, y, z), 5, chunk); }
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y < heightMap - rand() % 3) { setBlock(glm::vec3(x, y, z), 3, chunk); }
					}
				}
		}

		void setBlock(const glm::vec3& pos, const int& block, const int& chunk) {
			if (chunk >= world.size()) return;
			if (chunk < 0) return;
			int x = pos.x; x = x % chunkSize;
			int y = pos.y; y = y % chunkSize;
			int z = pos.z; z = z % chunkSize;
			world[chunk].chunkData[x][y][z] = block;
		}
		void setBlock(const glm::vec3& pos, const int& block) {
			int chunk = to1D({ GetChunkPos(pos.x) ,GetChunkPos(pos.y) ,GetChunkPos(pos.z) });
			if (chunk >= world.size()) return;
			if (chunk < 0) return;
			int x = pos.x; x = x % chunkSize;
			int y = pos.y; y = y % chunkSize;
			int z = pos.z; z = z % chunkSize;
			world[chunk].chunkData[x][y][z] = block;
		}

		void UpdateWorldMesh(const int& chunk) {
			if (chunk >= world.size()) return;
			if (chunk < 0) return;

			uint32_t offset = 0;
			world[chunk].IndexCount = 0;
			//Updating the fluid mesh
			world[chunk].fIndexCount = 0;
			uint32_t fOffset = 0;

			gl::Vertex worldMesh[MaxVertexCount];
			gl::Vertex worldFluidMesh[MaxVertexCount];

			for (int8_t y = 0; y < chunkSize; y++)
				for (int8_t z = 0; z < chunkSize; z++)
					for (int8_t x = 0; x < chunkSize; x++)
					{
						if (makeFace({x,y,z}, chunk, ConnectionType::CONNECT_DEFAULT)) // Can make block face
						{
							int block = world[chunk].chunkData[x][y][z];
							//Positive
							if (y + 1 < chunkSize) { 
								int checkBlock = world[chunk].chunkData[x][y + 1][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].TexCoords[(int)BlockTexture::TOP], world[chunk].IndexCount, offset, worldMesh);
							}
						
							else if (chunk + to1D({ 0,1,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], world[chunk].IndexCount, offset, worldMesh);
								}
							}
							
							if (z + 1 < chunkSize) { 
								int checkBlock = world[chunk].chunkData[x][y][z + 1];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 0,0,1 }) < world.size()){
								int checkBlock = world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], world[chunk].IndexCount, offset, worldMesh);
								}
							}
							
							if (x + 1 < chunkSize) { 
								int checkBlock = world[chunk].chunkData[x + 1][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 1,0,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], world[chunk].IndexCount, offset, worldMesh);
								}
							}
						
							//Negative	   																										 					  															    						 
							if (y - 1 >= 0) {
								int checkBlock = world[chunk].chunkData[x][y - 1][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], world[chunk].IndexCount, offset, worldMesh);
							}
							else  if (chunk - to1D({ 0,1,0 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize - 1][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], world[chunk].IndexCount, offset, worldMesh);
								}
							}
						
							if (z - 1 >= 0) { 
								int checkBlock = world[chunk].chunkData[x][y][z - 1];
								if (world[chunk].chunkData[x][y][z - 1] == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk - to1D({ 0,0,1 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize - 1];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], world[chunk].IndexCount, offset, worldMesh);
								}
							}
						
							if (x - 1 >= 0) { 
								int checkBlock = world[chunk].chunkData[x - 1][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk - to1D({ 1,0,0 }) >= 0) {
								int checkBlock = world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize - 1][y][z];
								if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) {
									addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], world[chunk].IndexCount, offset, worldMesh); }
							}
						}

						if (makeFace({ x,y,z }, chunk, ConnectionType::FLUID_CONNECT)) // Can make a fluid face
						{
							int block = world[chunk].chunkData[x][y][z];
							//Positive
							if (y + 1 < chunkSize) {
								int checkBlock = world[chunk].chunkData[x][y + 1][z];
								if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], world[chunk].IndexCount, offset, worldMesh);
							}
							else if (chunk + to1D({ 0,1,0 }) < world.size()) {
								int checkBlock = world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z];
								if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
									addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], world[chunk].IndexCount, offset, worldMesh);
							}
						}
					}
			worldIndexBuffer.Bind();
			if (!world[chunk].IndexCount == 0){ world[chunk].chunkMeshBuffer.Update(0, sizeof(worldMesh), &worldMesh);}
		
			if (!world[chunk].fIndexCount == 0) { world[chunk].chunkFluidMeshBuffer.Update(0, sizeof(worldFluidMesh), &worldFluidMesh); }
		}

		void addFace(const Face& face, const glm::vec3& pos, const glm::vec2& Coords, uint32_t& IndexCount, uint32_t& offset, gl::Vertex* mesh) {
			if (IndexCount >= MaxFaceCount * 6) {
				WC_ERROR("Memory overflow!");			
				return;
			}

			for (uint8_t i = 0; i < 4; i++) { mesh[i + offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteIndexCoords(Coords, { 32,32 })[i]); }
			IndexCount += 6;
			offset += 4;
		}

		bool makeFace(const glm::vec3& pos, const int& chunk, ConnectionType type) {
			if (pos.x >= chunkSize || pos.y >= chunkSize || pos.z >= chunkSize) return false;
			if (pos.x < 0 || pos.y < 0 || pos.z < 0) return false;
			if (chunk >= world.size()) return false;
			if (chunk < 0) return false;
			int x = pos.x;
			int y = pos.y;
			int z = pos.z;
			int block = world[chunk].chunkData[x][y][z];
			if (block > 0 && blockData[block].blockConnectionType == type) return true;
			return false;
		} // @TODO

		void LoadBlocks() {
			blockData.reserve(7);
			blockAtlas.load("assets/textures/block/blockAtlas.png");
			//testBlockAtlas.Create();
			//testBlockAtlas.AddTexture("assets/textures/block/blockAtlas.png");

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
		}

		int GetChunkPos(int i) {
			int junk = 0;
			if (i % chunkSize != 0) { junk = i % chunkSize; i = i - junk; return i / chunkSize; }
			else { return i / chunkSize; }
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

		void LoadEnivoirment() {
			glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
			worldShader.use();
			worldShader.setBool("fog", Fog);
			worldShader.setVec3("fogColor", fogColor);
			if(Fog)glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
			skybox.Create("scripts/skybox.lua", p.Far);
		}

		bool Fog = false;
		int water_level = 32;
		gl::Skybox skybox;

		gl::Shader worldShader;
		gl::Shader fluidShader;

		gl::Texture blockAtlas;
		gl::IndexBuffer worldIndexBuffer;
		std::unordered_map<int, Block> blockData;
		std::array<Chunk, 16 * 16 * 16> world;
	};
}