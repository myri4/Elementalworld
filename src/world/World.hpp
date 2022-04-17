#pragma once

#include <wc/pch.hpp>
#include "Chunk.hpp"
#include "Biome.hpp"
#include <Maths/Ray.hpp>
#include <Maths/Frustum.hpp>
#include <FastNoise/FastNoiseLite.h>
#include "../entities/Player.hpp"
#include <wc/Model/Animation.hpp>
#include "../Game Mechanics/LineBatcher.hpp"
#include "../Game Mechanics/CommandParser.hpp"
#include <Utils/YAML.hpp>
#include <Utils/Memory.h>
#include <ppl.h>

#define MODEL1

namespace wc {
	static const uint32_t RenderDistance = 16;

	enum class GameMsg : uint32_t
	{
		Server_GetStatus,
		Server_GetPing,

		Client_Accepted,
		Client_AssignID,
		Client_RegisterWithServer,
		Client_UnregisterWithServer,

		Game_AddPlayer,
		Game_RemovePlayer,
		Game_UpdatePlayer,

		RequestChunk,
		SendChunk,
		GenerateChunk,
		BlockEdit
	};

	class GameInstance {
		// Player related
		Camera camera;
		float MouseSensitivity = 5.f;
		float gravity = 20.f;

		LineBatcher lineBatcher;

		gl::Shader skyShader;
		gl::Buffer skyboxVertexBuffer;
		gl::VertexArray skyBoxArray;
		float rotateSpeed = 1.f * 6.f; // one cycle is one unit (in minutes)
		float angle = 0.f;

		Frustum viewFrustum;
		gl::Shader chunkShader;
		std::filesystem::file_time_type chunkShaderTimer;

		gl::UniformBuffer transforms;

		// Lighting
		gl::UniformBuffer lights;
		gl::UniformBuffer materials;
		uint32_t currentLightID = 0;
		bool lightUpdate = false;

		struct TransformData {
			glm::mat4 ViewProj = glm::mat4(1.f);
			alignas(16) glm::vec3 cameraPos = glm::vec3(0.f);
			alignas(16) glm::vec3 lower_left_corner = glm::vec3(0.f);
			alignas(16) glm::vec3 horizontal = glm::vec3(0.f);
			alignas(16) glm::vec3 vertical = glm::vec3(0.f);
			alignas(16) glm::vec2 windowSize = glm::vec2(0.f);
			uint32_t numLights = 0;
		};

		gl::VertexArray chunkMeshArray;
		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> chunks;

		FastNoiseLite worldNoise;
		FastNoiseLite temperatureNoise;
		FastNoiseLite moistureNoise;
		FastNoiseLite treeNoise;
		FastNoiseLite caveNoise;

		bool gnerateTerrain : 1;

		int8_t water_level = 0;
		uint32_t localPlayerID = 0;

		std::unordered_map<uint32_t, PlayerDescription> players;

		// World saving
		std::string worldName = "New world";

#ifdef MODEL
		gl::Shader modelShader;
		//Animation animation;
		Model model;
		glm::vec3 modelPos = { (RenderDistance * RenderDistance * 0.5f + RenderDistance), 51.f , (RenderDistance * RenderDistance * 0.5f) };
#endif

		void SaveStructure(const char* outFile, const glm::ivec3& Start, const glm::ivec3& End) {
			glm::ivec3 start = Start;
			glm::ivec3 end = End;

			if (start.x > end.x) std::swap(start.x, end.x);
			if (start.y > end.y) std::swap(start.y, end.y);
			if (start.z > end.z) std::swap(start.z, end.z);

			std::ofstream file(outFile, std::ios::binary | std::ios::out | std::ios::trunc);
			for (int y = start.y; y < end.y; y++)
				for (int x = start.x; x < end.x; x++)
					for (int z = start.z; z < end.z; z++)
					{
						BlockID blockID = getBlock({ x,y,z });
						if(blockID) file << (int)blockID << " " << x - start.x << " " << y - start.y << " " << z - start.z << "\n";
					}
			
			file.close();
		}

		void LoadStructure(const char* fileName, const glm::ivec3& offset) {
			std::ifstream file(fileName, std::ios::binary | std::ios::in);
			
			if (file) {
				int block = 0;
				glm::ivec3 pos;
				while (!file.eof()) {
					if (!file) break;
					file >> block >> pos.x >> pos.y >> pos.z;
					setBlock(pos + offset, block, false, false);
				}
			}
		}

		// Multiplayer
		bool bWaitingForConnection = true;
		net::client_interface<GameMsg> clientInstance;
		
		struct Light {
			glm::vec3 vector;
			uint32_t color;
		} lighting[chunkVolume];

		// Composite stuff
		gl::FrameBuffer screen;
		gl::Texture scrTexture;
		gl::Texture finalImage;
		gl::ComputeShader compositeShader;
		gl::Texture bloomBuffers[3];

		glm::ivec2 bloomTexSize = glm::ivec2(0);
		uint32_t m_BloomComputeWorkGroupSize = 4;
		uint32_t mips = 1;

		gl::ComputeShader bloomShader;
		gl::UniformBuffer bloomUBO;

		enum class BloomMode
		{
			Prefilter,
			Downsample,
			UpsampleFirst,
			Upsample
		};

		struct BloomUBOSettings {
			glm::vec4 Params = glm::vec4(1.f); // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
			float LOD = 0.f;
			int Mode = (int)BloomMode::Prefilter;
		};

		struct BloomSettings
		{
			float Threshold = 1.f;
			float Knee = 0.1f;
		}bloomSettings;
	public:
		bool multiPlayer = false;
		Player p;

		void Create() {
			chunkShader.Create("resourcepacks/default/shaders/chunkShader.glsl");
			chunkShaderTimer = std::filesystem::last_write_time("resourcepacks/default/shaders/chunkShader.glsl");
			bloomShader.Create("resourcepacks/default/shaders/bloomShader.glsl");
			compositeShader.Create("resourcepacks/default/shaders/composite.glsl");

			transforms.Create(sizeof(TransformData), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			transforms.BufferBase(0);

			lights.Create(sizeof(lighting), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			lights.BufferBase(1);

			bloomUBO.Create(sizeof(BloomUBOSettings), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			bloomUBO.BufferBase(4);

			sol::state worldGenState;
			worldGenState.new_usertype<FastNoiseLite>("Noise", sol::constructors<void()>(),
				"SetOctaves", &FastNoiseLite::SetFractalOctaves,
				"SetLacunarity", &FastNoiseLite::SetFractalLacunarity,
				"SetGain", &FastNoiseLite::SetFractalGain,
				"SetSeed", &FastNoiseLite::SetSeed,
				"SetFrequency", &FastNoiseLite::SetFrequency,
				"SetNoiseType", &FastNoiseLite::SetNoiseType,
				"SetFractalType", &FastNoiseLite::SetFractalType,
				"SetMultiplier", &FastNoiseLite::SetMultiplier
				);
			worldGenState.new_usertype<Block>("Block", sol::constructors<void()>(), "texture", &Block::texture, "ConnectionType", &Block::connectionType);
			//worldGenState.new_enum("FractalType", {{ "None", FastNoiseLite::FractalType::FractalType_None },{ "None", FastNoiseLite::FractalType::FractalType_None } });
			worldGenState.script_file("scripts/worldGen.lua");
			if (worldGenState["noise"].valid()) worldNoise = worldGenState["noise"];
			if (worldGenState["TempNoise"].valid()) temperatureNoise = worldGenState["TempNoise"];
			if (worldGenState["MoistureNoise"].valid()) moistureNoise = worldGenState["MoistureNoise"];
			if (worldGenState["TreeNoise"].valid()) treeNoise = worldGenState["TreeNoise"];
			if (worldGenState["CaveNoise"].valid()) caveNoise = worldGenState["CaveNoise"];

			if (worldGenState["water_level"].valid()) water_level = worldGenState["water_level"];

			assets.Create(30, 32, 32);
			Block airBlock;
			blockData.push_back(airBlock);
			Material mat;
			materialData.push_back(mat);
			Item noItem;
			itemData.push_back(noItem);

			//Loading blocks
			worldGenState.set_function("AddBlockScript", &AddBlockScript);
			worldGenState.script_file("scripts/blocks.lua");

			materials.Create(materialData.byte_size(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT, materialData.data());
			materials.BufferBase(3);

			worldGenState.new_usertype<Biome>("Biome", sol::constructors<void()>(),
				"maxMois", &Biome::maxMois,
				"maxTemp", &Biome::maxTemp,
				"minMois", &Biome::minMois,
				"minTemp", &Biome::minTemp,
				"topBlock", &Biome::topBlock
				);
			worldGenState.set_function("AddBiome", [&](const Biome& biome) { biomeMap.push_back(biome); });
			worldGenState.script_file("scripts/biomes.lua");
			assets.Free();

			lineBatcher.Create();
			chunkMeshArray.Create();
			chunkMeshArray.VertexAttribPointer(0, 3, offsetof(Vertex, Position));  // position attribute
			chunkMeshArray.VertexAttribPointer(1, 1, offsetof(Vertex, TexCoords)); // texture coord attribute
			chunkMeshArray.VertexAttribPointer(2, 3, offsetof(Vertex, Normal)); // type attribute
			chunkMeshArray.VertexAttribPointer(4, 1, offsetof(Vertex, materialID)); // color attribute
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				//Configuring the vertex buffer
				chunks[chunkID].meshBuffer.Create(MaxVertexCount * sizeof(Vertex), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
				chunks[chunkID].indexBuffer.Create(sizeof(uint32_t) * MaxFaceCount * 6, GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			}

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions 
				-1.f, -1.f,
				-1.f,  1.f,
				 1.f, -1.f,

				 1.f, -1.f,
				-1.f,  1.f,
				 1.f,  1.f,
			};

			skyboxVertexBuffer.Create(sizeof(quadVertices), 0, quadVertices);
			skyBoxArray.Create();
			skyBoxArray.VertexAttribPointer(0, 2, 0);
			skyBoxArray.AddVertexBuffer(skyboxVertexBuffer, sizeof(float) * 2);

			skyShader.Create("resourcepacks/default/shaders/skybox.glsl");
#ifdef MODEL
			modelShader.Create("resourcepacks/default/shaders/modelShader.glsl");
			model.Create("assets/models/Ravenkin.obj");
			//animation.Create("assets/models/dancing_vampire.dae", model);
#endif // MODEL

			addLight(glm::vec3(0.f), convertColor(glm::vec4(1.f, 1.f, 1.f, 0.f)));
			recipes[0].data[0] = 6; recipes[0].data[1] = 0;
			recipes[0].data[2] = 0; recipes[0].data[3] = 0;
			recipes[0].amount = 4;
			recipes[0].result = 18;

			recipes[1].data[0] = 18; recipes[1].data[1] = 18;
			recipes[1].data[2] = 18; recipes[1].data[3] = 18;
			recipes[1].amount = 1;
			recipes[1].result = 19;

			//BlockMeshes.Load("assets/models/seagrass.ply");
			//blockData[18].connectionType = ConnectionType::CUSTOM_MODEL;

			grass = getBlockID("grass_block");
			stone = getBlockID("stone_block");

			const uint32_t xMeshIndexArray[] = {
				0, 1, 2,
				2, 3, 0,

				4, 5, 6,
				6, 7, 4
			};

			BlockMesh& xMesh = blockMeshes[0];
			xMesh.indices.reserve(ARRAYSIZE(xMeshIndexArray));
			for(int i = 0; i < ARRAYSIZE(xMeshIndexArray); i++)
				xMesh.indices.emplace_back(xMeshIndexArray[i]);

			//WC_INFO("albedo: {0}", offsetof(Material, albedo));
			//WC_INFO("normal: {0}", offsetof(Material, normal));
			//WC_INFO("MRA: {0}", offsetof(Material, MRA));
			//WC_INFO("flags: {0}", offsetof(Material, flags));
			//WC_INFO("color: {0}", offsetof(Material, color));
			
			glm::vec3 normal1 = glm::vec3( 0.70710677f, 0.f, 0.70710677f);
			glm::vec3 normal2 = glm::vec3(-0.70710677f, 0.f, 0.70710677f);
			xMesh.vertices.reserve(8);
			xMesh.vertices.emplace_back(Vertex(glm::vec3(blockSize,  blockSize, 0.f),  glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal1, 0));
			xMesh.vertices.emplace_back(Vertex(glm::vec3(blockSize, 0.f, 0.f),         glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal1, 0));
			xMesh.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f,  blockSize),        glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal1, 0));
			xMesh.vertices.emplace_back(Vertex(glm::vec3(0.f,  blockSize,  blockSize), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal1, 0));

			xMesh.vertices.emplace_back(Vertex(glm::vec3(blockSize,  blockSize,  blockSize), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal2, 0));
			xMesh.vertices.emplace_back(Vertex(glm::vec3(blockSize, 0.f,  blockSize),        glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal2, 0));
			xMesh.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 0.f),                     glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal2, 0));
			xMesh.vertices.emplace_back(Vertex(glm::vec3(0.f,  blockSize, 0.f),              glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, normal2, 0));

			const uint32_t slabMeshIndexArray[] = {
				0, 1, 2,
				2, 3, 0,

				4, 5, 6,
				6, 7, 4,

				8, 9, 10,
				10, 11, 8,

				12, 13, 14,
				14, 15, 12,

				16, 17, 18,
				18, 19, 16,
				
				20, 21, 22,
				22, 23, 20
			};

			BlockMesh& slabMeshDown = blockMeshes[1];
			slabMeshDown.indices.reserve(ARRAYSIZE(slabMeshIndexArray));
			for (int i = 0; i < ARRAYSIZE(slabMeshIndexArray); i++)
				slabMeshDown.indices.emplace_back(slabMeshIndexArray[i]);

			slabMeshDown.vertices.reserve(24);
			// Top
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 1.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.5f, 1.f), glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 1.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.5f, 1.f), glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 1.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.5f, 0.f), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 1.f, 0.f), 0));

			// Bottom
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, -1.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, -1.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, -1.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, -1.f, 0.f), 0));

			// Front
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.5f, 1.f), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, -1.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, -1.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, -1.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.5f, 1.f), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, -1.f), 0));

			// Back
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, 1.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, 1.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, 1.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.5f, 0.f), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(0.f, 0.f, 1.f), 0));

			// Left
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.5f, 1.f), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(1.f, 0.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(1.f, 0.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(1.f, 0.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(0.f, 0.5f, 0.f), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(1.f, 0.f, 0.f), 0));

			// Right
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(-1.f, 0.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(-1.f, 0.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(-1.f, 0.f, 0.f), 0));
			slabMeshDown.vertices.emplace_back(Vertex(glm::vec3(1.f, 0.5f, 1.f), glm::vec3(1.f, 0.f, 0.f) * 255.f, (uint8_t)ConnectionType::CUSTOM_MODEL, glm::vec3(-1.f, 0.f, 0.f), 0));
		}

		// Common blocks
		BlockID grass = 0;
		BlockID stone = 0;

		void Join(const std::string& ip, const std::string& playerName) {
			if (multiPlayer) {
				clientInstance.Connect(ip, 60000);
				p.name = playerName;
			}
			LoadWorld();
		}

		void CreateScreen() {
			// Creating the screen framebuffer
			gl::TextureProps scrProps;
			scrProps.internalFormat = GL_RGBA32F;
			scrProps.min_filter = GL_LINEAR_MIPMAP_LINEAR;
			scrProps.mag_filter = GL_LINEAR;
			scrProps.wrap_s = GL_CLAMP_TO_EDGE;
			scrProps.wrap_t = GL_CLAMP_TO_EDGE;
			scrProps.SetSize(window.GetSize());
			scrTexture.Create(scrProps);
			finalImage.Create(scrProps);

			screen.Create(scrProps.Width, scrProps.Height);
			screen.addTexture(scrTexture);

			bloomTexSize = glm::ivec2(scrProps.Width, scrProps.Height) / 2;
			bloomTexSize += glm::ivec2(m_BloomComputeWorkGroupSize - bloomTexSize.x % m_BloomComputeWorkGroupSize, m_BloomComputeWorkGroupSize - bloomTexSize.y % m_BloomComputeWorkGroupSize);
			mips = scrTexture.GetMipLevelCount() - 4;
			gl::TextureProps bloomProps;
			bloomProps.internalFormat = GL_RGBA32F;
			bloomProps.mips = mips;
			bloomProps.min_filter = GL_LINEAR_MIPMAP_LINEAR;
			bloomProps.mag_filter = GL_LINEAR;
			bloomProps.wrap_s = GL_CLAMP_TO_EDGE;
			bloomProps.wrap_t = GL_CLAMP_TO_EDGE;

			bloomProps.SetSize(bloomTexSize);
			for (int i = 0; i < 3; i++) {
				bloomBuffers[i].Create(bloomProps);
				bloomBuffers[i].GenerateMipMap();
			}
		}

		void DestroyScreen() {
			screen.Destroy();
			scrTexture.Destroy();
			finalImage.Destroy();

			for (int i = 0; i < 3; i++)
				bloomBuffers[i].Destroy();
		}

		uint32_t getScreen() { return finalImage; }
		
		void Update(const float& deltaTime) {
			screen.Bind();
			Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// Multiplayer 
			if (multiPlayer) {
				if (clientInstance.IsConnected())
				{
					while (!clientInstance.Incoming().empty())
					{
						auto msg = clientInstance.Incoming().pop_front().msg;

						switch (msg.header.id)
						{
						case(GameMsg::Client_Accepted):
						{
							net::message<GameMsg> msg;
							msg.header.id = GameMsg::Client_RegisterWithServer;
							PlayerDescription descPlayer;
							msg << descPlayer;
							clientInstance.Send(msg);
							break;
						}

						case(GameMsg::Client_AssignID):
						{
							// Server is assigning us OUR id
							msg >> localPlayerID;
							WC_INFO("Assigned Client ID = {0}", localPlayerID);
							break;
						}

						case(GameMsg::Game_AddPlayer):
						{
							PlayerDescription desc;
							msg >> desc;
							players.insert_or_assign(desc.nUniqueID, desc);
							if (desc.nUniqueID == localPlayerID)
							{
								p.Position = desc.Position;
								p.rotation = desc.rotation;
								p.currentSlot = desc.currentSlot;
								p.health = desc.health;
								// Now we exist in game world
								bWaitingForConnection = false;
							}
							break;
						}

						case(GameMsg::Game_RemovePlayer):
						{
							uint32_t nRemovalID = 0;
							msg >> nRemovalID;
							players.erase(nRemovalID);
							break;
						}

						case(GameMsg::Game_UpdatePlayer):
						{
							PlayerDescription desc;
							msg >> desc;
							players.insert_or_assign(desc.nUniqueID, desc);
							break;
						}

						case(GameMsg::BlockEdit):
						{
							glm::ivec3 pos = glm::ivec3(0);
							BlockID block = 0;
							msg >> pos >> block;
							setBlock(pos, block, true, false);
							break;
						}
						case(GameMsg::SendChunk):
						{
							std::vector<std::pair<BlockID, uint16_t>> data;
							ChunkID chunkID = 0;
							uint32_t size = 0;
							msg >> chunkID >> size;
							data.reserve(size);
							for (uint32_t i = 0; i < size; i++) {
								std::pair<BlockID, uint16_t> pair;
								msg >> pair;
								data.emplace_back(pair);
							}
							Decompress(data, chunkID);
							break;
						}
						case(GameMsg::GenerateChunk):
						{
							uint16_t chunkID = 0;
							msg >> chunkID;
							chunks[chunkID].generated = false;
							chunks[chunkID].generatedStructures = false;
							chunks[chunkID].canBeUpdated = true;
							gnerateTerrain = true;
							break;
						}
						}
					}
					// @WARN: glEnable(GL_DEPTH_TEST) might be needed
					for (auto& player : players) {
						if (player.second.nUniqueID != localPlayerID) {
							DrawOutlineCube(player.second.Position - p.Size, p.Size * 2.f, glm::vec4(1.f));
						}
					}
				}

				if (bWaitingForConnection) WC_INFO("Waiting for connection");// Tell the client you are waiting

				net::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_UpdatePlayer;
				players[localPlayerID].Position = p.Position;
				players[localPlayerID].currentSlot = p.currentSlot;
				players[localPlayerID].health = p.health;
				players[localPlayerID].name = p.name;
				players[localPlayerID].rotation = p.rotation;
				msg << players[localPlayerID];
				clientInstance.Send(msg);				
			}

			// camera/view transformation
			TransformData data;
			data.windowSize = window.GetSize();
			data.ViewProj = glm::perspective(glm::radians(camera.FOV), data.windowSize.x / data.windowSize.y, 0.1f, 1100.f) * camera.GetViewMatrix();
			data.cameraPos = camera.Position;
			data.lower_left_corner = camera.lower_left_corner;
			data.vertical = camera.vertical;
			data.horizontal = camera.horizontal;
			data.numLights = currentLightID;

			lighting[0].vector = -glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)));
			transforms.SetData(0, sizeof(TransformData), &data);
			if (lightUpdate) {
				lights.SetData(0, sizeof(Light) * currentLightID, lighting);
				lightUpdate = false;
			}
			else 
				lights.SetData(0, sizeof(Light), lighting);			

			// Draw sky
			skyShader.use();
			skyBoxArray.Bind();

			Renderer::DrawArrays(6);

			angle += deltaTime * rotateSpeed;
			angle = glm::mod(angle, 360.f);
			glEnable(GL_DEPTH_TEST);

			viewFrustum.update(data.ViewProj);
			uint32_t chunkHalf = chunkSize / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position); // @TODO: hmmm? why doesnt it work with glm::ivec3?

			for (ChunkID i = 0; i < chunks.size(); i++) {
				glm::ivec3& currChunkPos = chunks[i].position;
				if (currChunkPos.x < currentPlayerPos.x - chunkHalf) ResetChunk(i, glm::ivec3(currentPlayerPos.x + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > currentPlayerPos.x + chunkHalf) ResetChunk(i, glm::ivec3(currentPlayerPos.x - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				if (currChunkPos.y < currentPlayerPos.y - chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currentPlayerPos.y + chunkHalf - 1, currChunkPos.z));
				if (currChunkPos.y > currentPlayerPos.y + chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currentPlayerPos.y - chunkHalf + 1, currChunkPos.z));

				if (currChunkPos.z < currentPlayerPos.z - chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z + chunkHalf - 1));
				if (currChunkPos.z > currentPlayerPos.z + chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z - chunkHalf + 1));
			}

			if (gnerateTerrain) {
				for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
					if (!chunks[chunk].generated) { GenerateChunkTerrain(chunk); chunks[chunk].generated = true; }

				for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
					if (!chunks[chunk].generatedStructures) { GenerateChunkStructures(chunk); chunks[chunk].generatedStructures = true; }

				gnerateTerrain = false;
			}

			assets.Bind();
			chunkShader.use();
			for (ChunkID i = 0; i < chunks.size(); i++) {

				if (chunks[i].canBeUpdated) { UpdateMesh(chunks[i]); chunks[i].canBeUpdated = false; }
			
				if (!chunks[i].empty && viewFrustum.isBoxInFrustum(AABB(chunks[i].position * glm::ivec3(chunkSize), glm::vec3(chunkSize)))) {
					chunkMeshArray.AddVertexBuffer(chunks[i].meshBuffer, sizeof(Vertex));
					chunkMeshArray.AddIndexBuffer(chunks[i].indexBuffer);
					chunkMeshArray.Bind();
					Renderer::DrawIndexed(chunks[i].IndexCount);
				}
			}
			//DrawOtlineCube(sStart, sEnd - sStart, glm::vec4(1.f));
			if (thirdPerson)
				DrawOutlineCube(p.Position - p.Size, p.Size * 2.f, glm::vec4(2.f));
			lineBatcher.Flush();
#ifdef MODEL
			modelShader.use();

			//modelShader.setMat4Array(1, animation.GetPoseTransforms(), MAX_BONE_WEIGHTS);
			
			// render the loaded model
			//animation.Update(deltaTime);
			glm::mat4 Model = glm::mat4(1.f);
			Model = glm::translate(Model, modelPos);    // translate it down so it's at the center of the scene
			Model = glm::scale(Model, glm::vec3(0.3f));
			modelShader.setMat4(0, Model);
			glDisable(GL_BLEND);
			model.Draw();
			glEnable(GL_BLEND);
#endif
			screen.unbind();
			// GUI
			glDisable(GL_DEPTH_TEST);

			RenderBloom();

			finalImage.BindTextureImage(0, GL_WRITE_ONLY);
			scrTexture.Bind(1); // use the color attachment texture as the texture of the quad plane	
			bloomBuffers[2].Bind(2);
			compositeShader.use();
			compositeShader.Dispatch(glm::ceil((glm::vec2)data.windowSize / glm::vec2(m_BloomComputeWorkGroupSize)));
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			Renderer2D::DrawQuad({ 0,0 }, data.windowSize, finalImage);
		}

		glm::vec3 m_rayEnd;
		glm::vec3 m_rayStart;
		bool thirdPerson = false;

		void ParseCommand(std::string& command) {
			// Command parsing
			std::string args;
			CommandType commandType = getCommandType(command, args);
			args += ' ';
			if (commandType == CommandType::textMessage) WC_INFO(command);
			else if (commandType == CommandType::fly) {
				p.flying = getArgument(args);
				p.wasOnGround = false;
				p.m_isOnGround = false;
				p.wasFalling = false;
			}
			else if (commandType == CommandType::collide) p.collision = getArgument(args);
			else if (commandType == CommandType::setBlock) setBlock({ getArgument(args, 1) , getArgument(args, 2) , getArgument(args, 3) }, getArgument(args), true, true);
			//else if (commandType == CommandType::give) p.inventory.AddItem(getArgument(args, 0), 0, getArgument(args, 1));
			else if (commandType == CommandType::setSpeed) p.MovementSpeed = getArgument(args, 0);
			else if (commandType == CommandType::setTime) angle = getArgument(args, 0);
			else if (commandType == CommandType::getBlockID) WC_INFO(getBlock({ getArgument(args, 0) , getArgument(args, 1) , getArgument(args, 2) }));
			else if (commandType == CommandType::capture) {
				glm::vec3 startPos = p.Position - p.Size;
				glm::vec3 endPos = p.Position + p.Size;
				WC_INFO("Normal pos: {0} {1} {2}", (int)startPos.x, (int)startPos.y, (int)startPos.z);
				WC_INFO("Normal pos: {0} {1} {2}", (int)endPos.x, (int)endPos.y, (int)endPos.z);

				p.Position = -p.Position;

				startPos = p.Position - p.Size;
				endPos = p.Position + p.Size;
				WC_INFO("Flipped pos: {0} {1} {2}", (int)startPos.x, (int)startPos.y, (int)startPos.z);
				WC_INFO("Flipped pos: {0} {1} {2}", (int)endPos.x, (int)endPos.y, (int)endPos.z);

				p.Position = -p.Position;
			}
#ifdef MODEL
			else if (commandType == CommandType::tpModel)
				modelPos = p.Position;
#endif
			else if (commandType == CommandType::UNKNOWN) WC_ERROR("Unknow command!");
			command = "";
		}

		void OnInput(const float& deltaTime) {
			glm::ivec2 windSize = window.GetSize();
			// MENU MANAGMENT
			if (std::filesystem::last_write_time("resourcepacks/default/shaders/chunkShader.glsl") != chunkShaderTimer) {
				chunkShader.Destroy();
				chunkShader.Create("resourcepacks/default/shaders/chunkShader.glsl");
				chunkShaderTimer = std::filesystem::last_write_time("resourcepacks/default/shaders/chunkShader.glsl");
			}

			//if (wc::bReloadModelShader) {
#ifdef MODEL
			//	modelShader.Destroy();
			//	modelShader.Create("shaderpacks/default/modelShader.glsl");
#endif // MODEL
			//	wc::bReloadModelShader = false;
			//}

			if (/*!textbox.isSelected*/true) {

				if (Keyboard::getKey(Keyboard::Key::Left)) camera.Roll += 0.5f;
				if (Keyboard::getKey(Keyboard::Key::Right)) camera.Roll -= 0.5f;

				// GAMEPLAY
				float yaw = glm::radians(p.rotation.x);
				float yaw90 = glm::radians(p.rotation.x + 90.f);
				float addFOV = 0.f;
				if (Keyboard::isKeyPressed(Keyboard::Key::W)) { // Front
					float adder = 0.f;
					if (Keyboard::isKeyPressed(Keyboard::Key::LControl)) { adder = 40.f; /*addFOV = 10.f;*/ }
					else if (Keyboard::isKeyPressed(Keyboard::Key::LShift) && !p.flying) { adder = -2.f; }
					p.acceleration.x += glm::cos(yaw) * (p.MovementSpeed + adder);
					p.acceleration.z += glm::sin(yaw) * (p.MovementSpeed + adder);
				}

				else if (Keyboard::isKeyPressed(Keyboard::Key::S)) { // Back
					p.acceleration.x -= glm::cos(yaw) * p.MovementSpeed;
					p.acceleration.z -= glm::sin(yaw) * p.MovementSpeed;
				}
				if (Keyboard::isKeyPressed(Keyboard::Key::A)) { // Left
					p.acceleration.x -= glm::cos(yaw90) * p.MovementSpeed;
					p.acceleration.z -= glm::sin(yaw90) * p.MovementSpeed;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::D)) { // Right
					p.acceleration.x += glm::cos(yaw90) * p.MovementSpeed;
					p.acceleration.z += glm::sin(yaw90) * p.MovementSpeed;
				}

				if (Keyboard::getKey(Keyboard::Key::F5) == GLFW_PRESS && !thirdPerson) {
					camera.distanceFromCamera = 3.f;
					thirdPerson = true;
				}
				else if (Keyboard::getKey(Keyboard::Key::F5) == GLFW_PRESS && thirdPerson) {
					camera.distanceFromCamera = 0.f;
					thirdPerson = false;
				}

				if (Keyboard::isKeyPressed(Keyboard::Key::Space))
				{
					if (!p.flying) {
						if (p.m_isOnGround)
						{
							p.acceleration.y = p.MovementSpeed * 2.f;
							p.m_isOnGround = false;
						}
					}
					else p.acceleration.y += p.MovementSpeed;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::LShift) && p.flying)
					p.acceleration.y -= p.MovementSpeed;

				if (Keyboard::isKeyPressed(Keyboard::Key::C)) { addFOV = -80.f; MouseSensitivity = 18; }
				else
					MouseSensitivity = 5;

				camera.FOV = addFOV + 90.f;

				if (scrollY != 0.f) {
					if (scrollY < 0) p.currentSlot++;
					else p.currentSlot--;
					if (p.currentSlot < 0) p.currentSlot = inventorySizeX - 1;
					else if (p.currentSlot > inventorySizeX - 1) p.currentSlot = 0;
				}

				int16_t xt, yt;

				glm::ivec2 pos = Mouse::GetMousePosToWindow();

				xt = windSize.x / 2;
				yt = windSize.y / 2;

				float ms = 1.f / MouseSensitivity;

				p.rotation.x -= (xt - pos.x) * ms;
				p.rotation.y += (yt - pos.y) * ms;

				// make sure that when pitch is out of bounds, screen doesn't get flipped
				if (p.rotation.y > 89.f)p.rotation.y = 89.f;
				else if (p.rotation.y < -89.f)p.rotation.y = -89.f;

				if (p.rotation.x > 360.f) p.rotation.x = 0.f;
				else if (p.rotation.x < 0.f) p.rotation.x = 360.f;

				Mouse::SetMousePosition(xt, yt);
			}

			// PLAYER RELATED
			p.velocity += p.acceleration;
			p.acceleration = { 0.f,0.f,0.f };

			if (!p.flying)
				p.velocity.y -= gravity * deltaTime;

			float velocityY = p.velocity.y;
			if (p.collision) {
				p.Position.x += p.velocity.x * deltaTime;
				collide({ p.velocity.x,0.f,0.f });
				p.Position.y += p.velocity.y * deltaTime;
				collide({ 0.f,p.velocity.y,0.f });
				p.Position.z += p.velocity.z * deltaTime;
				collide({ 0.f,0.f,p.velocity.z });
			}
			else p.Position += p.velocity * deltaTime;

			if (!p.wasFalling && p.isFalling()) p.startOfFall = p.Position.y;
			if (!p.wasOnGround && p.m_isOnGround) 
			if (p.startOfFall - p.Position.y > minFallDistance) p.health -= 1.5f;

			
			p.wasOnGround = p.m_isOnGround;
			p.wasFalling = p.isFalling();

			camera.Position = p.Position;
			camera.Position.y += p.Size.y - 0.1f;
			camera.Yaw = p.rotation.x;
			camera.Pitch = p.rotation.y;

			p.velocity.x *= 0.009f;
			p.velocity.z *= 0.009f;
			if (p.flying)
				p.velocity.y *= 0.009f;
			//////////////

			camera.UpdateCameraAngles();

			bool bBreak = Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)  == GLFW_PRESS;
			bool bPlace = Mouse::getMouse(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

			glm::vec3 m_rayLastPos;
			bool bShow = true;
			m_rayStart = p.Position;
			m_rayEnd = m_rayStart;
			
			while (glm::length(p.Position - m_rayEnd) < 6.f)
			{
				m_rayEnd += camera.Front * 0.05f;
				glm::ivec3 pos = static_cast<glm::ivec3>(m_rayEnd);

				BlockID block = getBlock(pos);
				if (block > 0 && block != 5)
				{	
					if (bShow) {
						bShow = false;
						DrawOutlineCube(pos, glm::vec3(1.f), glm::vec4(3.f));
						if (bBreak) {
							//p.inventory.AddItem(block - 1, p.currentSlot);
							setBlock(pos, 0, true, true);
						}
						else if (bPlace) {
							//ItemID itemID = p.inventory.data[p.currentSlot].itemID;
							//if (p.inventory.RemoveItem(p.currentSlot))
								setBlock(m_rayLastPos, /*items[itemID].block*/1, true, true);
						}
						break;
					}
				}
				m_rayLastPos = pos;
			}
		}

		void Destroy() {
			if (multiPlayer) clientInstance.Disconnect();
			else SaveWorld();
		}

	private:

		void collide(const glm::vec3& vel) {
			glm::vec3 startPos = p.Position - p.Size;
			glm::vec3 endPos = p.Position + p.Size;

			for (int x = startPos.x; x < endPos.x; x++)
			for (int y = startPos.y; y < endPos.y; y++)
			for (int z = startPos.z; z < endPos.z; z++) {
				BlockID blockID = getBlock(glm::ivec3(x, y, z));
				Block block = blockData[blockID];
				if (blockID != 0u && block.isCollidable) {
					if (vel.y > 0) {
						p.Position.y = y - p.Size.y;
						p.velocity.y = 0.f;
					}
					else if (vel.y < 0) {
						p.m_isOnGround = true;
						p.Position.y = y + p.Size.y + blockSize;
						p.velocity.y = 0.f;
					}

					if (vel.x > 0)
						p.Position.x = x - p.Size.x;
					else if (vel.x < 0)
						p.Position.x = x + p.Size.x + blockSize;

					if (vel.z > 0)
						p.Position.z = z - p.Size.z;
					else if (vel.z < 0)
						p.Position.z = z + p.Size.z + blockSize;
				}
			}
		}		

		std::string getChunkPath(const glm::ivec3& pos) {
			return "worlds/" + worldName + "/Chunk data/Island 0/r." + std::to_string(pos.x) + "." + std::to_string(pos.y) + "." + std::to_string(pos.z) + ".ewr";
		}

		// SERIALIZATION/DESERIALIZATION
		void CreateNewWorld(const std::string& name) {
			worldName = name;
			std::filesystem::create_directories("worlds/" + worldName +"/Chunk data/Island 0");
			std::filesystem::create_directories("worlds/" + worldName + "/Entity data");
			SaveWorld();
			SavePlayerState(p);
		}

		void LoadWorld() {
			p.Position = { 0, (RenderDistance * RenderDistance * 0.5f), 0 };
			LoadPlayerState(p, p.name);
			glm::ivec3 offset = getChunkPos(p.Position) - glm::ivec3(chunkSize / 2);
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				chunks[chunkID].position = to3D(chunkID, glm::ivec3(RenderDistance)) + offset;

				UpdateNeighbours(chunkID);
				TryToLoadChunk(chunkID);
			}
			if (!multiPlayer) {
				YAML::Node config = YAML::LoadFile("worlds/" + worldName + "/world.properties");
				if (config["time"]) angle = config["time"].as<float>();
				if (config["seed"]) worldNoise.SetSeed(config["seed"].as<int>());
			}
		}

		void SaveWorld() {
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) SaveChunk(chunkID);

			YAML::Node config;
			config["seed"] = worldNoise.GetSeed();
			config["time"] = angle;
			YAMLUtils::saveFile("worlds/" + worldName + "/world.properties", config);

			SavePlayerState(p);
		}

		void TryToLoadChunk(const ChunkID& chunkID) {
			if (multiPlayer && clientInstance.IsConnected()) {
				net::message<GameMsg> msg;
				msg.header.id = GameMsg::RequestChunk;
				glm::ivec3 position = chunks[chunkID].position;
				msg << position << chunkID;
				clientInstance.Send(msg);
			}
			else {
				std::ifstream file(getChunkPath(chunks[chunkID].position), std::ios::binary | std::ios::in);
				if (file.is_open()) {
					std::vector<std::pair<BlockID, uint16_t>> data;

					while (!file.eof()) {
						uint32_t block;
						uint16_t count;

						file >> block >> count;
						data.emplace_back(block, count);
					}

					data[data.size() - 1] = { 0,0 };
					file.close();

					Decompress(data, chunkID);
				}
				else {
					chunks[chunkID].generated = false;
					chunks[chunkID].generatedStructures = false;
					gnerateTerrain = true;
				}
			}
		}

		void SaveChunk(const ChunkID& chunkID) {
			if (!multiPlayer) {
				if (chunks[chunkID].used) {

					std::ofstream file(getChunkPath(chunks[chunkID].position), std::ios::binary | std::ios::out | std::ios::trunc);
					auto data = Compress(chunkID);
					for (auto& block : data) {
						int32_t blockID = block.first;
						uint16_t count = block.second;
						file << blockID << " " << count << "\n";
					}

					file.close();
					chunks[chunkID].used = false;
				}
			}
		}

		void SavePlayerState(const Player& player) {
			YAML::Node config;
			config["MovementSpeed"] = player.MovementSpeed;
			config["rotation"] = player.rotation;
			config["currentSlot"] = (uint32_t)player.currentSlot;
			config["position"] = player.Position;
			config["flying"] = (uint32_t)player.flying;
			config["collision"] = (uint32_t)player.collision;
			config["velocity"] = player.velocity;
			config["acceleration"] = player.acceleration;
			config["health"] = player.health;
			YAMLUtils::saveFile("worlds/" + worldName + "/Entity data/" + player.name + ".ec", config);
		}

		void LoadPlayerState(Player& player, const std::string& playerName) {
			if (!std::filesystem::exists("worlds/" + worldName + "/Entity data/" + playerName + ".ec")) {
				Player pl;
				SavePlayerState(pl);
				player = pl;
			}
			else {
				YAML::Node config = YAML::LoadFile("worlds/" + worldName + "/Entity data/" + playerName + ".ec");
				if (config["MovementSpeed"]) player.MovementSpeed = config["MovementSpeed"].as<float>();
				if (config["rotation"])      player.rotation = config["rotation"].as<glm::vec2>();
				if (config["currentSlot"])   player.currentSlot = config["currentSlot"].as<uint32_t>();
				if (config["position"])      player.Position = config["position"].as<glm::vec3>();
				if (config["flying"])        player.flying = config["flying"].as<uint32_t>();
				if (config["collision"])     player.collision = config["collision"].as<uint32_t>();
				if (config["velocity"])      player.velocity = config["velocity"].as<glm::vec3>();
				if (config["acceleration"])  player.acceleration = config["acceleration"].as<glm::vec3>();
				if (config["health"])        player.health = config["health"].as<float>();
			}
		}		

		// WORLD GENERATION		
		// VERY TEMPORARLY!!
		void GenerateTree(const int& x, const int& y, const int& z, const float& treeHeight, const ChunkID& chunk) {
			int32_t trunkHeight = (int32_t)(treeHeight * 2.f) + 6;
			
			for(int x2 = x - 2; x2 < x + 3; x2++)
				for (int y2 = y - 3 + trunkHeight - 1; y2 < y + 2 + trunkHeight - 1; y2++)
					for (int z2 = z - 2; z2 < z + 3; z2++)
						setBlock(glm::ivec3(x2, y2, z2) + (int)chunkSize * chunks[chunk].position, 9, false, false);					
			
			for (int i = 0; i < trunkHeight; i++) 
				setBlock(glm::ivec3(x, y + i, z) + (int)chunkSize * chunks[chunk].position, 7, false, false);
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {
				memset(&chunks[chunk].data, 0, sizeof(chunks[chunk].data));
				concurrency::parallel_for(uint32_t(0), (uint32_t)chunkSize, [&](uint32_t z){
					for (uint32_t x = 0; x < chunkSize; x++) {
						glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].position.x * chunkSize, z + chunks[chunk].position.z * chunkSize);
						//int heightMap = (int)worldNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
						//float floraGen = treeNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y) * 0.5f + 0.5f;
						//float baseTemperature = temperatureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
						//float moisture = moistureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y) * 0.5f + 0.5f;
						//int dirtDepth = (int)(floraGen * 3.f) + 2;

						for (uint8_t y = 0; y < chunkSize; y++) {
								glm::ivec3 pos = chunks[chunk].position * glm::ivec3(chunkSize) + glm::ivec3(x, y, z);
								//float temperature = baseTemperature * (1.f - pos.y / (worldNoise.GetMultiplier() - water_level));
								//uint32_t biome = getBiome(temperature, moisture);
								//bool onSand = (pos.y <= water_level + (int)(floraGen * 3.f) && pos.y == heightMap);
								//if (pos.y == heightMap) {
								//	if (onSand)
								//		setBlock(glm::ivec3(x, y, z), 4, chunk);
								//	else 
								//		setBlock(glm::ivec3(x, y, z), biomeMap[biome].topBlock, chunk);
								//}
								//
								//if (pos.y < heightMap && pos.y >= heightMap - dirtDepth) setBlock(glm::ivec3(x, y, z), 2, chunk);
								//if (pos.y < heightMap - dirtDepth) setBlock(glm::ivec3(x, y, z), 3, chunk);	
								//if (pos.y > heightMap && pos.y < water_level) setBlock(glm::ivec3(x, y, z), 5, chunk);	
								if (pos.y == 125) setBlockLocal(glm::ivec3(x, y, z), grass, chunk, false, false);
						}
					}
					return;
				});
		}

		void GenerateChunkStructures(const ChunkID& chunk) {
			//concurrency::parallel_for(ChunkID(0), (ChunkID)chunkSize, [&](ChunkID z) {
			//	for (uint8_t x = 0; x < chunkSize; x++) {
			//		glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].position.x * chunkSize, z + chunks[chunk].position.z * chunkSize);
			//		int heightMap = (int)worldNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
			//		float treeGen = treeNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
			//		float baseTemperature = temperatureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
			//
			//		for (uint8_t y = 0; y < chunkSize; y++) {
			//			glm::ivec3 pos = chunks[chunk].position * glm::ivec3(chunkSize) + glm::ivec3(x, y, z);
			//			uint32_t type = getBiome(0.75f, 0.f);
			//			float CaveNoise = caveNoise.GetNoise((float)pos.x, (float)pos.y, (float)pos.z);
			//			bool onGrass = !(pos.y <= water_level + (int)(treeGen * 2.f) && pos.y == heightMap);
			//
			//			if (pos.y == heightMap + 1 && heightMap + 1 > water_level && onGrass && !(CaveNoise >= 0.25f && CaveNoise <= 0.99f))
			//					if (treeGen <= 0.49f && treeGen > 0.48f && x % 5 == 0 && biomeMap[type].trees) GenerateTree(x, y, z, treeGen, chunk);
			//		}
			//	}
			//	return;
			//});
		}

		uint32_t getBiome(const float& temperature, const float& moisture = 0.f) {
			for (uint32_t i = 1; i < biomeMap.size(); i++) {
				if (temperature >= biomeMap[i].minTemp && temperature <= biomeMap[i].maxTemp
					//&& moisture >= biomeMap[i].minMois && moisture <= biomeMap[i].maxMois
					) return i;
			}
			return 0;
		}

		// LIGHT MANAGING
		uint32_t addLight(const glm::vec3& position, const uint32_t& color) {
			uint32_t light = currentLightID;
			if (currentLightID <= ARRAYSIZE(lighting)) {
				lighting[currentLightID].vector = position;
				lighting[currentLightID].color = color;
				currentLightID++;
				lightUpdate = true;
			}
			return light;
		}

		void removeLight(const glm::vec3& position) {
			for (uint32_t i = 0u; i < ARRAYSIZE(lighting); i++)
				if (lighting[i].vector == position) {
					currentLightID--;
					lighting[i] = lighting[currentLightID];
					lightUpdate = true;
					break;
				}
		}

		// WORLD/CHUNK MANAGING
		void setBlockLocal(const glm::ivec3& pos, const BlockID& block, const ChunkID& chunk, const bool& playerEdited, const bool& replyToServer) {
			uint16_t x = pos.x;
			uint16_t y = pos.y;
			uint16_t z = pos.z;

			BlockID& chunkBlock = chunks[chunk].data[x][y][z];
			if (block == 0 && blockData[chunkBlock].emitLight)
				removeLight((glm::vec3)chunks[chunk].position * (float)(chunkSize) + (glm::vec3)pos + glm::vec3(0.5f));
			else if (blockData[block].emitLight) 
				addLight((glm::vec3)chunks[chunk].position * (float)chunkSize + (glm::vec3)pos + glm::vec3(0.5f), convertColor(glm::vec4(1.f)));			

			chunks[chunk].data[x][y][z] = block;
			chunks[chunk].canBeUpdated = true;
			chunks[chunk].empty = chunks[chunk].empty && block == 0;

			if (replyToServer && multiPlayer) {
				net::message<GameMsg> msg;
				msg.header.id = GameMsg::BlockEdit;
				msg << block << pos + chunks[chunk].position * glm::ivec3(chunkSize);
				clientInstance.Send(msg);
			}
			else if (playerEdited) chunks[chunk].used = true;

			if (x == 0) { int16_t neg = chunks[chunk].neighborNeg[0]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (y == 0) { int16_t neg = chunks[chunk].neighborNeg[1]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (z == 0) { int16_t neg = chunks[chunk].neighborNeg[2]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }

			if (x == chunkSize - 1) { int16_t Pos = chunks[chunk].neighborPos[0]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (y == chunkSize - 1) { int16_t Pos = chunks[chunk].neighborPos[1]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (z == chunkSize - 1) { int16_t Pos = chunks[chunk].neighborPos[2]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block, const bool& playerEdited, const bool& replyToServer) {
			int16_t chunk = getChunkID(getChunkPos(pos));
			if (chunk > -1) setBlockLocal(getBlockPos(pos), block, chunk, playerEdited, replyToServer);
		}	

		void UpdateMesh(Chunk& chunk) {
			uint32_t offset = 0;
			chunk.IndexCount = 0;
			uint32_t ioffset = 0;

			glm::ivec3 chunkPos = chunk.position * glm::ivec3(chunkSize);
			Vertex* vertices = (Vertex*)chunk.meshBuffer.Map(GL_WRITE_ONLY);
			uint32_t* indices = (uint32_t*)chunk.indexBuffer.Map(GL_WRITE_ONLY);

			const uint32_t indexArray[] = {
				0, 1, 2,
				2, 3, 0
			};

			bool done = false;
			uint32_t i = 0, j = 0, k = 0, l = 0, w = 0, h = 0, d = 0, u = 0, v = 0, n = 0;
			uint8_t type = ConnectionType::NON_EXISTENT, checkType = ConnectionType::NON_EXISTENT;
			BlockID mask[chunkSize * chunkSize];
			uint32_t textureMask[chunkSize * chunkSize];
			BlockID blockID = 0, checkBlock = 0;
			glm::ivec3 x = glm::ivec3(0);
			glm::ivec3 q = glm::ivec3(0);
			glm::ivec3 du = glm::ivec3(0);
			glm::ivec3 dv = glm::ivec3(0);
			// ChunkLogic
			for (uint32_t x = 0; x < chunkSize; x++)
				for (uint32_t y = 0; y < chunkSize; y++)
					for (uint32_t z = 0; z < chunkSize; z++) {
						if (y + 1 < chunkSize && chunk.data[x][y][z] == 0) {
							if (chunk.data[x][y + 1][z] == 14 || chunk.data[x][y + 1][z] == 10
								|| chunk.data[x][y + 1][z] == 15 || chunk.data[x][y + 1][z] == 13) chunk.data[x][y + 1][z] = 0;
						}
					}
			// Sweep over each axis (X, Y and Z)
			for (d = 0; d < 3; d++)
			{
				i = 0, j = 0, k = 0, l = 0, w = 0, h = 0;
				u = (d + 1) % 3;
				v = (d + 2) % 3;
				x[u] = 0;
				x[v] = 0;
				x[d] = -1;
			
				memset(mask, 0, sizeof(mask));
				memset(textureMask, 0, sizeof(textureMask));
				q[d] = 1;
				q[u] = 0;
				q[v] = 0;
			
				// Check each slice of the chunk one at a time
				for (; x[d] < chunkSize;)
				{
					// Compute the mask
					n = 0;
					for (x[v] = 0; x[v] < chunkSize; x[v]++)
					{
						for (x[u] = 0; x[u] < chunkSize; x[u]++)
						{
							// q determines the direction (X, Y or Z) that we are searching
							// m.IsBlockAt(x,y,z) takes global map positions and returns true if a block exists there
							blockID = 0;
							type = ConnectionType::NON_EXISTENT;
							checkBlock = 0;
							checkType = ConnectionType::NON_EXISTENT;

							if (x[d] >= 0) {
								blockID = chunk.data[x[0]][x[1]][x[2]];
								type = blockData[blockID].connectionType;
							}

							glm::ivec3 xq = x + q;
							if (xq[d] < chunkSize) {
								checkBlock = chunk.data[xq.x][xq.y][xq.z];
								checkType = blockData[checkBlock].connectionType;
							}
							else if(chunk.neighborPos[d] > -1) {
								xq[d] = 0;
								checkBlock = chunks[chunk.neighborPos[d]].data[xq.x][xq.y][xq.z];
								checkType = blockData[checkBlock].connectionType;
							}
							// The mask is set to true if there is a visible face between two blocks, i.e. both aren't empty and both aren't blocks

							if (type == ConnectionType::CUSTOM_MODEL) {
								if (chunk.IndexCount < MaxFaceCount * 6) {
									Block& block = blockData[blockID];

									if (blockMeshes[block.meshID].optimize) {

									}
									else {
										for (uint32_t i = 0; i < blockMeshes[block.meshID].vertices.size(); i++) {
											glm::vec2 texCoords = convertColor(blockMeshes[block.meshID].vertices[i].TexCoords);
											vertices[i + offset] = Vertex(
												blockMeshes[block.meshID].vertices[i].Position + glm::vec3(chunkPos + x),
												{ texCoords, block.texture[(int)BlockTexture::TOP] }, block.connectionType, 
												blockMeshes[block.meshID].vertices[i].Normal, 0);
										}

										for (uint32_t i = 0; i < blockMeshes[block.meshID].indices.size(); i++)
											indices[chunk.IndexCount + i] = blockMeshes[block.meshID].indices[i] + ioffset;
									}

									ioffset += blockMeshes[block.meshID].vertices.size();
									chunk.IndexCount += blockMeshes[block.meshID].indices.size();
									offset += blockMeshes[block.meshID].vertices.size();									
								}
							}
							
							if (type != checkType && type != ConnectionType::NON_EXISTENT && checkType != ConnectionType::NON_EXISTENT) {
								if (blockID != checkBlock && type == ConnectionType::CONNECT_DEFAULT) { mask[n] = blockID + 1; textureMask[n] = blockData[blockID].texture[d]; }
								else if ((blockID == 0 || type != ConnectionType::CONNECT_DEFAULT) && checkType == ConnectionType::CONNECT_DEFAULT) { mask[n] = checkBlock + 1; textureMask[n] = blockData[checkBlock].texture[d + 3]; }

								else if (blockID != checkBlock && type == ConnectionType::NO_CONNECT) { mask[n] = blockID + 1; textureMask[n] = blockData[blockID].texture[d]; }
								else if (blockID == 0 && checkType == ConnectionType::NO_CONNECT) { mask[n] = checkBlock + 1; textureMask[n] = blockData[checkBlock].texture[d + 3]; }

								else if (checkBlock == 0 && type == ConnectionType::FLUID_CONNECT) { mask[n] = blockID + 1; textureMask[n] = blockData[blockID].texture[d]; }
								else if (blockID == 0 && checkType == ConnectionType::FLUID_CONNECT) { mask[n] = checkBlock + 1; textureMask[n] = blockData[checkBlock].texture[d + 3]; }
							}
							n++;
						}
					}
			
					x[d]++;
			
					n = 0;
			
					// Generate a mesh from the mask using lexicographic ordering,      
					//   by looping over each block in this slice of the chunk
					for (j = 0; j < chunkSize; j++)
					{
						for (i = 0; i < chunkSize;)
						{
							if (textureMask[n])
							{
								// Compute the width of this quad and store it in w                        
								//   This is done by searching along the current axis until mask[n + w] is false
								for (w = 1; i + w < chunkSize && textureMask[n + w] && textureMask[n + w] == textureMask[n]; w++) {}
			
								// Compute the height of this quad and store it in h                        
								//   This is done by checking if every block next to this row (range 0 to w) is also part of the mask.
								//   For example, if w is 5 we currently have a quad of dimensions 1 x 5. To reduce triangle count,
								//   greedy meshing will attempt to expand this quad out to CHUNK_SIZE x 5, but will stop if it reaches a hole in the mask
								done = false;
								for (h = 1; j + h < chunkSize; h++)
								{
									// Check each block next to this quad
									for (k = 0; k < w; ++k)
									{
										// If there's a hole in the mask, exit
										if (!textureMask[n + k + h * chunkSize] || textureMask[n + k + h * chunkSize] != textureMask[n])
										{
											done = true;
											break;
										}
									}
			
									if (done)
										break;
								}
			
								x[u] = i;
								x[v] = j;
			
								// du and dv determine the size and orientation of this face
								du[u] = w;
								du[d] = 0;
								du[v] = 0;

								dv[v] = h;
								dv[d] = 0;
								dv[u] = 0;
								float h1 = h, w1 = w;
								// Create a quad for this face. Colour, normal or textures are not stored in this block vertex format.
								Face face;
								if (d == 0) {
									face.corner[2] = x;           // Top-left vertice position
									face.corner[3] = x + du;      // Top right vertice position
									face.corner[1] = x + dv;      // Bottom left vertice position
									face.corner[0] = x + du + dv; // Bottom right vertice position
								}
								else if (d == 1) {
									face.corner[0] = x;                 // Top-left vertice position
									face.corner[1] = x + du;         // Top right vertice position
									face.corner[3] = x + dv;         // Bottom left vertice position
									face.corner[2] = x + du + dv; // Bottom right vertice position
								}
								else if (d == 2) {
									face.corner[2] = x;                 // Top-left vertice position
									face.corner[1] = x + du;         // Top right vertice position
									face.corner[3] = x + dv;         // Bottom left vertice position
									face.corner[0] = x + du + dv; // Bottom right vertice position
									std::swap(h1, w1);
								}
								face.CalculateNormal();
								if (!(chunk.IndexCount >= MaxFaceCount * 6)) {
									BlockID blockID = mask[n] - 1;
									glm::vec2 TexCoords[] = {
										glm::vec2(0.f, 0.f),
										glm::vec2(0.f, w1),
										glm::vec2(h1,  w1),
										glm::vec2(h1,  0.f),
									};

									for (uint32_t i = 0; i < ARRAYSIZE(face.corner); i++)
										vertices[i + offset] = Vertex(face.corner[i] + (glm::vec3)chunkPos, { TexCoords[i], textureMask[n] }, blockData[blockID].connectionType, face.normal, blockData[blockID].material);

									for (uint32_t i = 0; i < ARRAYSIZE(indexArray); i++)
										indices[chunk.IndexCount + i] = indexArray[i] + ioffset;

									ioffset += ARRAYSIZE(face.corner);
									chunk.IndexCount += ARRAYSIZE(indexArray);
									offset += ARRAYSIZE(face.corner);
								}
								// Clear this part of the mask, so we don't add duplicate faces
								for (l = 0; l < h; l++)
									for (k = 0; k < w; k++) {
										mask[n + k + l * chunkSize] = 0;
										textureMask[n + k + l * chunkSize] = 0;
									}

								// Increment counters and continue
								i += w;
								n += w;
							}
							else
							{
								i++;
								n++;
							}
						}
					}
				}
			}
			chunk.meshBuffer.UnMap();
			chunk.indexBuffer.UnMap();
			chunk.empty = chunk.IndexCount == 0;
		}
		 
		BlockID getBlock(const glm::ivec3& pos) {
			int16_t chunk = getChunkID(getChunkPos(pos));
			if (chunk < 0) return 0;
			glm::ivec3 blockPos = getBlockPos(pos);
			return chunks[chunk].data[blockPos.x][blockPos.y][blockPos.z];
		}

		int16_t getChunkID(const glm::ivec3& pos) {
			for (ChunkID i = 0; i < chunks.size(); i++) {
					if (chunks[i].position == pos) 
						return i;				
			}
			return -1;
		}

		void ResetChunk(const ChunkID& chunkID, const glm::ivec3& newChunkPos) {
			SaveChunk(chunkID);
			chunks[chunkID].position = newChunkPos;
			UpdateNeighbours(chunkID);
			TryToLoadChunk(chunkID);
		}

		void UpdateNeighbours(const ChunkID& chunk) {
			glm::ivec3 neighborXpos = chunks[chunk].position + glm::ivec3(1, 0, 0);
			glm::ivec3 neighborYpos = chunks[chunk].position + glm::ivec3(0, 1, 0);
			glm::ivec3 neighborZpos = chunks[chunk].position + glm::ivec3(0, 0, 1);

			glm::ivec3 neighborXneg = chunks[chunk].position - glm::ivec3(1, 0, 0);
			glm::ivec3 neighborYneg = chunks[chunk].position - glm::ivec3(0, 1, 0);
			glm::ivec3 neighborZneg = chunks[chunk].position - glm::ivec3(0, 0, 1);

			if (chunks[chunks[chunk].neighborPos[0]].position != neighborXpos) { if (chunks[chunk].neighborPos[0] != -1) chunks[chunks[chunk].neighborPos[0]].neighborNeg[0] = -1; chunks[chunk].neighborPos[0] = -1; }
			if (chunks[chunks[chunk].neighborPos[1]].position != neighborYpos) { if (chunks[chunk].neighborPos[1] != -1) chunks[chunks[chunk].neighborPos[1]].neighborNeg[1] = -1; chunks[chunk].neighborPos[1] = -1; }
			if (chunks[chunks[chunk].neighborPos[2]].position != neighborZpos) { if (chunks[chunk].neighborPos[2] != -1) chunks[chunks[chunk].neighborPos[2]].neighborNeg[2] = -1; chunks[chunk].neighborPos[2] = -1; }

			if (chunks[chunks[chunk].neighborNeg[0]].position != neighborXneg) { if (chunks[chunk].neighborNeg[0] != -1) chunks[chunks[chunk].neighborNeg[0]].neighborPos[0] = -1; chunks[chunk].neighborNeg[0] = -1; }
			if (chunks[chunks[chunk].neighborNeg[1]].position != neighborYneg) { if (chunks[chunk].neighborNeg[1] != -1) chunks[chunks[chunk].neighborNeg[1]].neighborPos[1] = -1; chunks[chunk].neighborNeg[1] = -1; }
			if (chunks[chunks[chunk].neighborNeg[2]].position != neighborZneg) { if (chunks[chunk].neighborNeg[2] != -1) chunks[chunks[chunk].neighborNeg[2]].neighborPos[2] = -1; chunks[chunk].neighborNeg[2] = -1; }

			concurrency::parallel_for(ChunkID(0), (ChunkID)chunks.size(), [&](ChunkID i) {
				if (chunks[i].position == neighborXpos) { chunks[chunk].neighborPos[0] = i; chunks[chunks[chunk].neighborPos[0]].neighborNeg[0] = chunk; }
				if (chunks[i].position == neighborYpos) { chunks[chunk].neighborPos[1] = i; chunks[chunks[chunk].neighborPos[1]].neighborNeg[1] = chunk; }
				if (chunks[i].position == neighborZpos) { chunks[chunk].neighborPos[2] = i; chunks[chunks[chunk].neighborPos[2]].neighborNeg[2] = chunk; }

				if (chunks[i].position == neighborXneg) { chunks[chunk].neighborNeg[0] = i; chunks[chunks[chunk].neighborNeg[0]].neighborPos[0] = chunk; }
				if (chunks[i].position == neighborYneg) { chunks[chunk].neighborNeg[1] = i; chunks[chunks[chunk].neighborNeg[1]].neighborPos[1] = chunk; }
				if (chunks[i].position == neighborZneg) { chunks[chunk].neighborNeg[2] = i; chunks[chunks[chunk].neighborNeg[2]].neighborPos[2] = chunk; }

				if (chunks[chunk].neighborPos[0] != -1 && chunks[chunk].neighborPos[1] != -1 && chunks[chunk].neighborPos[2] != -1 &&
					chunks[chunk].neighborNeg[0] != -1 && chunks[chunk].neighborNeg[1] != -1 && chunks[chunk].neighborNeg[2] != -1) return;
				});
		}

		BlockID getBlockID(const std::string& name) {
			for (BlockID i = 0; i < blockData.size(); i++)
				if (blockData[i].name == name) return i;

			WC_WARN("Could not find {0}. Air block assumed.", name);
			return 0;
		}

		// MISC
		std::vector<std::pair<BlockID, uint16_t>> Compress(const ChunkID& chunk) {
			std::vector<std::pair<BlockID, uint16_t>> compressed;
			BlockID pBlockID = chunks[chunk].data[0][0][0];
			uint16_t count = 0;

			for (uint8_t y = 0; y < chunkSize; y++)
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) // @TODO: optimize
					{
						BlockID block = chunks[chunk].data[x][y][z];
						if (block == pBlockID) count++;
						else {
							compressed.emplace_back(pBlockID, count);
							pBlockID = chunks[chunk].data[x][y][z];
							count = 1;
						}
					}
			compressed.emplace_back(pBlockID, count);
			return compressed;
		}

		void Decompress(const std::vector<std::pair<BlockID, uint16_t>>& blocks, const ChunkID& chunk)
		{
			uint16_t counter = 0;
			for (auto& block : blocks) {
				for (uint16_t i = 0; i < block.second; i++) {
					glm::ivec3 pos = to3D(counter, glm::ivec3(chunkSize));
					uint8_t x = pos.x;
					uint8_t y = pos.z;
					uint8_t z = pos.y;
					setBlockLocal({ x,y,z }, block.first, chunk, false, false);
					counter++;
				}
			}
		}

		void DrawOutlineCube(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color) {
			lineBatcher.DrawLine(pos,								   pos + glm::vec3(0.f,    size.y, 0.f),    color);
			lineBatcher.DrawLine(pos,								   pos + glm::vec3(size.x, 0.f,    0.f),    color);
			lineBatcher.DrawLine(pos + glm::vec3(size.x, 0.f,    0.f), pos + glm::vec3(size.x, size.y, 0.f),    color);
			lineBatcher.DrawLine(pos + glm::vec3(size.x, size.y, 0.f), pos + glm::vec3(0.f,    size.y, 0.f),    color);
																									        
			lineBatcher.DrawLine(pos + glm::vec3(0.f, 0.f,    size.z), pos + glm::vec3(0.f,    size.y, size.z), color);
			lineBatcher.DrawLine(pos + glm::vec3(0.f, 0.f,    size.z), pos + glm::vec3(size.x, 0.f,    size.z), color);
			lineBatcher.DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, size.y, size.z), color);
			lineBatcher.DrawLine(pos + size,                           pos + glm::vec3(0.f,    size.y, size.z), color);

			lineBatcher.DrawLine(pos + glm::vec3(0.f, 0.f,    size.z), pos,                                     color);
			lineBatcher.DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, 0.f, 0.f),       color);

			lineBatcher.DrawLine(pos + glm::vec3(0.f, size.y, size.z), pos + glm::vec3(0.f,    size.y, 0.f),	color);
			lineBatcher.DrawLine(pos + size,						   pos + glm::vec3(size.x, size.y, 0.f),	color);
		}

		// BLOOM
		void RenderBloom()
		{
			bloomShader.use();
			BloomUBOSettings settings;
			settings.Params = glm::vec4(bloomSettings.Threshold, bloomSettings.Threshold - bloomSettings.Knee, bloomSettings.Knee * 2.f, 0.25f / bloomSettings.Knee);
			bloomUBO.SetData(0, sizeof(BloomUBOSettings), &settings);
			bloomBuffers[0].BindTextureImage(0, GL_WRITE_ONLY);
			scrTexture.Bind(1);
			bloomShader.Dispatch(glm::ceil(glm::vec2(bloomTexSize) / glm::vec2(m_BloomComputeWorkGroupSize)));
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			bloomBuffers[0].Bind(1);
			for (int currentMip = 1; currentMip < mips; currentMip++)
			{
				glm::vec2 mipSize = bloomBuffers[0].GetMipSize(currentMip);
				mipSize = glm::ceil(mipSize / glm::vec2(m_BloomComputeWorkGroupSize));
				settings.Mode = (int)BloomMode::Downsample;

				// Ping 
				settings.LOD = currentMip - 1;
				bloomUBO.SetData(0, sizeof(BloomUBOSettings), &settings);

				bloomBuffers[1].BindTextureImage(0, GL_WRITE_ONLY, currentMip);
				bloomShader.Dispatch(mipSize);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				// Pong 
				settings.LOD = currentMip;
				bloomUBO.SetData(0, sizeof(BloomUBOSettings), &settings);

				bloomBuffers[0].BindTextureImage(0, GL_WRITE_ONLY, currentMip);
				bloomBuffers[1].Bind(1);
				bloomShader.Dispatch(mipSize);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}

			// First Upsample		
			settings.LOD = mips - 2;
			settings.Mode = (int)BloomMode::UpsampleFirst;
			bloomUBO.SetData(0, sizeof(BloomUBOSettings), &settings);

			bloomBuffers[2].BindTextureImage(0, GL_WRITE_ONLY, mips - 1);
			bloomBuffers[0].Bind(1);

			bloomShader.Dispatch(glm::ceil((glm::vec2)bloomBuffers[2].GetMipSize(mips - 1) / glm::vec2(m_BloomComputeWorkGroupSize)));
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			bloomBuffers[2].Bind(2);
			settings.Mode = (int)BloomMode::Upsample;
			for (int currentMip = mips - 2; currentMip >= 0; currentMip--)
			{
				settings.LOD = currentMip;
				bloomUBO.SetData(0, sizeof(BloomUBOSettings), &settings);

				bloomBuffers[2].BindTextureImage(0, GL_WRITE_ONLY, currentMip);

				bloomShader.Dispatch(glm::ceil((glm::vec2)bloomBuffers[2].GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize)));
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}
		}
	};	
}