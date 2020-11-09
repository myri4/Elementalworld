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
		gl::ParticleEffect testEffect;
		gl::ParticleProps props;

		World() {}
		~World() {}

		void Create() {
			worldShader.Create("shaderpacks/default/core.glsl");
			worldShader.use();
			worldShader.setInt("u_Texture", 0);

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

			p.InitPlayer(glm::vec3(0, chunkSize * 4, 0));
			for (int i = 0; i < world.size(); i++) {
				world[i].Create(i);
			
				GenerateChunkTerrain(i);
				UpdateWorldMesh(i);
			}

			gl::Material material;

			material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
			material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
			material.shininess = 0.0f;
			material.Apply(worldShader, "material");
		}

		void Update(const sf::RenderWindow& window, const bool& CenterMouse, const float& deltaTime) {
			p.UpdatePlayer({ window.getPosition().x, window.getPosition().y }, CenterMouse, deltaTime);

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("u_Projection", p.projection);
			blockAtlas.Bind();

			// camera/view transformation
			worldShader.setMat4("u_View", p.GetView());

			Draw();
			//testEffect.Emit(props);
			//testEffect.OnUpdate(deltaTime);
			//testEffect.Render(p.projection);
			if (!Fog) 
				skybox.Draw(glm::mat4(glm::mat3(p.GetView())), p.projection);			
		}

		void OnEvent(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
			
			if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON || wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) {
				glm::vec3 m_rayLastPoint = glm::vec3(0.0f);
				Ray ray(p.camera.Position);
				for (; ray.getLength() < 8 * 6; ray.Step(p.camera.Yaw, p.camera.Pitch)) {
					if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON) {
						int chunk = to1D({ GetChunkPos(ray.getEnd().x) ,GetChunkPos(ray.getEnd().y) ,GetChunkPos(ray.getEnd().z) });
						setBlock(ray.getEnd(), 0, chunk); UpdateWorldMesh(chunk);
						break;
					}
					if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON) { 
						int chunk = to1D({ GetChunkPos(m_rayLastPoint.x) ,GetChunkPos(m_rayLastPoint.y) ,GetChunkPos(m_rayLastPoint.z)});
						setBlock(m_rayLastPoint, 1, chunk); UpdateWorldMesh(chunk);
						break;
					}
					m_rayLastPoint = ray.getEnd();
				}
			}
		}

	private:

		void Draw() {
			for (uint32_t i = 0; i < world.size(); i++) {
				if (world[i].chunkMeshBuffer.GetVAO() == 0 || worldIndexBuffer.GetEBO() == 0) return;
				// calculate the model matrix for each object and pass it to shader before drawing
				worldShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), to3D(world[i].chunkPosition) * glm::vec3(chunkSize)));

				Flush(i);
			}
		}

		void GenerateChunkTerrain(const int& chunk) {
			for (uint8_t z = 0; z < chunkSize; z++)
				for (uint8_t x = 0; x < chunkSize; x++) {
					int heightMap = worldNoise.getNoiseFor(glm::vec2(x, z), glm::vec2(to3D(world[chunk].chunkPosition).x, to3D(world[chunk].chunkPosition).z));
					for (int y = 0; y < chunkSize; y++) {
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
						if ((int)to3D(world[chunk].chunkPosition).y * chunkSize + y < heightMap - rand() % 3) { setBlock(glm::vec3(x, y, z), 3, chunk); }
					}
				}
		}

		void setBlock(const glm::vec3& pos, const int& block, const int& chunk) {
			if (pos.x >= chunkSize || pos.y >= chunkSize || pos.z >= chunkSize) return;
			if (pos.x < 0 || pos.y < 0 || pos.z < 0) return;
			if (chunk >= world.size()) return;
			if (chunk < 0) return;
			int x = pos.x;
			int y = pos.y;
			int z = pos.z;
			world[chunk].chunkData[x][y][z] = block;
		}

		void Flush(const int& chunk) {
			if (world[chunk].IndexCount <= 0) return;
			worldIndexBuffer.Bind();
			world[chunk].chunkMeshBuffer.Bind();
			world[chunk].chunkMeshBuffer.BindVBO();
			glDrawElements(GL_TRIANGLES, world[chunk].IndexCount, GL_UNSIGNED_INT, nullptr);
		}

		void UpdateWorldMesh(const int& chunk) {
			if (chunk >= world.size()) return;
			if (chunk < 0) return;
			offset = 0;
			world[chunk].IndexCount = 0;
			for (uint32_t i = 0; i < MaxVertexCount; i++) worldMesh[i] = gl::Vertex(glm::vec3(0.0f), glm::vec2(0.0f)); // Reseting the mesh
			for (int8_t y = 0; y < chunkSize; y++)
				for (int8_t z = 0; z < chunkSize; z++)
					for (int8_t x = 0; x < chunkSize; x++)
					{
						if (makeFace({x,y,z}, chunk))
						{
							//Positive
							if (y + 1 < chunkSize) { if (world[chunk].chunkData[x][y + 1][z] == 0) addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], chunk); }
							else  if (chunk + to1D({ 0,1,0 }) < world.size()) if (world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z] == 0) { addFace(TOP_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], chunk); }
							
							if (z + 1 < chunkSize) { if (world[chunk].chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], chunk); }
							else if(chunk + to1D({ 0,0,1 }) < world.size()) if (world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0] == 0) { addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], chunk); }
							
							if (x + 1 < chunkSize) { if (world[chunk].chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], chunk); }
							else if (chunk + to1D({ 1,0,0 }) < world.size()) if (world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z] == 0) { addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], chunk); }
			
							//Negative	   																										 					  															    						 
							if (y - 1 >= 0) { if (world[chunk].chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], chunk); }
							else  if (chunk - to1D({ 0,1,0 }) >= 0) if (world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize - 1][z] == 0) { addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], chunk); }
			
							if (z - 1 >= 0) { if (world[chunk].chunkData[x][y][z - 1] == 0) addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], chunk); }
							else if (chunk - to1D({ 0,0,1 }) >= 0) if (world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize - 1] == 0) { addFace(BACK_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], chunk); }
			
							if (x - 1 >= 0) { if (world[chunk].chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], chunk); }
							else if (chunk - to1D({ 1,0,0 }) >= 0) if (world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize - 1][y][z] == 0) { addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], chunk); }
						}
					}
			world[chunk].chunkMeshBuffer.Update(0, sizeof(worldMesh), worldMesh);
			//Flush(chunk);
		}

		void addFace(const Face& face, const glm::vec3& pos, const glm::vec2& Coords, const int& chunk) {
			if (world[chunk].IndexCount >= MaxFaceCount * 6) {
				WC_ERROR("Memory overflow!");			
				return;
			}

			for (uint8_t i = 0; i < 4; i++) { worldMesh[i + offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteIndexCoords(Coords, { 32,32 })[i]); }
			world[chunk].IndexCount += 6;
			offset += 4;
		}

		bool makeFace(const glm::vec3& pos, const int& chunk) {
			int x = pos.x;
			int y = pos.y;
			int z = pos.z;
			if (world[chunk].chunkData[x][y][z] > 0) return true;
			return false;
		} // @TODO

		void LoadBlocks() {
			blockData.reserve(3);
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
		}

		int GetChunkPos(int i) {
			int junk;
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
			worldShader.setBool("fog", Fog);
			worldShader.setVec3("fogColor", fogColor);
			glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
			skybox.Create("scripts/skybox.lua", p.Far);
		}

		bool Fog = false;
		gl::Shader worldShader;
		gl::Skybox skybox;
		Noise worldNoise;
		gl::Vertex worldMesh[MaxVertexCount];
		uint32_t offset = 0;
		gl::Texture blockAtlas;
		std::unordered_map<int, Block> blockData;
		std::array<Chunk, chunkSize * chunkSize * chunkSize> world;
		gl::IndexBuffer worldIndexBuffer;
	};

}