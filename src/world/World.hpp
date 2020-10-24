#pragma once

#include "Chunk.hpp"
#include "../entityes/Player.hpp"

namespace wc {
		
	struct NoiseOptions {
		int octaves;
		float amplitude;
		float smoothness;
		float roughness;
		float offset;
	};

	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const NoiseOptions& options, int seed) {
		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		float value = 0.0f;
		float accumulatedAmps = 0;
		value = glm::simplex(glm::vec2(voxelX / 32.0f + seed, voxelZ / 32.0f + seed));

		value = (value + 1) / 2;

		return value *= 5;
	}

	void AddBlock() {
		Block block;
		block.Create("scripts/grassblock.lua");
		blockData.push_back(block);
	}

	class World : public NonCopyable {
	public:
		//std::unordered_map<glm::vec3, Chunk> world;
		std::array<Chunk, 3> world;
		
		World(){
		
		}
		~World() {

		}

		void Create() {
			worldShader.Create("shaderpacks/default/core.glsl");
			worldShader.use();
			worldShader.setInt("u_Texture", 0);

			LoadBlocks();
			LoadEnivoirment();

			skybox.Create("scripts/skybox.lua");
			for (int i = 0; i < world.size(); i++) {
				world[i].Create(glm::vec3(i, 0, 0));
		
				for (uint8_t z = 0; z < chunkSize; z++) 
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap = getNoiseFor(glm::vec2(x, z), glm::vec2(world[i].chunkPosition.x, world[i].chunkPosition.z), worldNoiseOptions, 0);
						for (int y = 0; y < chunkSize; y++)
							if ((int)world[i].chunkPosition.y + y == heightMap) world[i].setBlock(glm::vec3(x,y,z), 1);
					}
			}
		}

		void Update(Player &p) {

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("projection", p.projection);
			blockAtlas.Bind();

			// camera/view transformation
			worldShader.setMat4("view", p.GetView());
			Draw();
			//skybox.Draw(glm::mat4(glm::mat3(p.GetView())), p.projection);
		}

	private:
		void Draw() {
			for (int i = 0; i < world.size(); i++)
				world[i].Draw(worldShader);
		}

		void LoadBlocks() {
			blockAtlas.load("assets/textures/block/blockAtlas.png");
			grassBlock.Create("scripts/grassblock.lua");
			grassBlock.material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.shininess = 32.0f;
			grassBlock.SendMaterialToShader(worldShader, "material");
			grassBlock.TexCoords[TOP_TEXTURE] = {0, 1};
			grassBlock.TexCoords[BOTTOM_TEXTURE] = { 1, 0 };
			grassBlock.TexCoords[LEFT_TEXTURE] = { 0, 0 };
			grassBlock.TexCoords[RIGHT_TEXTURE] = { 0, 0 };
			grassBlock.TexCoords[FRONT_TEXTURE] = { 0, 0 };
			grassBlock.TexCoords[BACK_TEXTURE] = { 0, 0 };
		}

		void LoadEnivoirment() {

		}
		gl::Shader worldShader;
		gl::Skybox skybox;
		NoiseOptions worldNoiseOptions;
	};

}