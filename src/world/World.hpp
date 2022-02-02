#pragma once

#include <wc/pch.hpp>
#include "Chunk.hpp"
#include "Biome.hpp"
#include <Maths/Ray.hpp>
#include <Maths/Frustum.hpp>
#include <FastNoise/FastNoiseLite.h>
#include "../entities/Player.hpp"
#include <wc/Model/Animation.hpp>
#include <wc/Skybox.hpp>
#include "../Game Mechanics/LineBatcher.hpp"
#include "../Game Mechanics/CommandParser.hpp"
#include <GUI/Console.hpp>
#include <GUI/Button.hpp>
#include <Utils/YAML.hpp>
#include <ppl.h>

#define MODEL1

namespace wc {
	static const uint32_t RenderDistance = 16;	
	Block blockData[256];
	uint32_t currentTexture = 0;
	uint8_t currentBiomeID = 0;
	Biome biomeMap[256];

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

	class Singleplayer {
	private:
		// Player related
		Camera camera;
		float MouseSensitivity = 5.f;
		float gravity = 20.f;

		LineBatcher lineBatcher;

		gl::Skybox skybox;
		Frustum viewFrustum;
		gl::Shader chunkShader;
		gl::UniformBuffer transforms;

		// Lighting
		gl::UniformBuffer lights;
		uint32_t currentLightID = 0;
		bool lightUpdate = false;

		struct TransformData {
			glm::mat4 proj = glm::mat4(1.f);
			glm::mat4 view = glm::mat4(1.f);
			float dt = 0.f;
			uint32_t numLights = 0;
			alignas(16) glm::vec3 cameraPos = glm::vec3(0.f);
			alignas(16) glm::vec3 lower_left_corner = glm::vec3(0.f);
			alignas(16) glm::vec3 horizontal = glm::vec3(0.f);
			alignas(16) glm::vec3 vertical = glm::vec3(0.f);
			alignas(16) glm::vec2 windowSize = glm::vec2(0.f);
			alignas(16) glm::vec3 fogColor = glm::vec3(0.f);
			float u_Density = 0.f;// 0.007f;
			float u_Gradient = 1.5f;
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
		PlayerDescription descPlayer;

		//GUI
		Console console;
		Textbox textbox;

		// World saving
		std::string worldName = "New world";

#ifdef MODEL
		gl::Shader modelShader;
		//Animation animation;
		Model model;
		glm::vec3 modelPos = { (RenderDistance * RenderDistance * 0.5f + RenderDistance), 51.f , (RenderDistance * RenderDistance * 0.5f) };
#endif

		static void AddBlockScript(const char* script) {
			std::string conType;
			sol::state blockState;
			blockState.script_file(script);
			BlockID blockID = 0;
			if (blockState["id"].valid()) blockID = blockState["id"];

			Block& block = blockData[blockID];

			if (blockState["isCollidable"].valid()) block.isCollidable = blockState["isCollidable"];
			if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];
			if (blockState["color"].valid()) block.color = blockState["color"];

			if (conType == "CONNECT_DEFAULT")    block.connectionType = ConnectionType::CONNECT_DEFAULT;
			else if (conType == "FLUID_CONNECT") block.connectionType = ConnectionType::FLUID_CONNECT;
			else if (conType == "NO_CONNECT")    block.connectionType = ConnectionType::NO_CONNECT;
			else if (conType == "X_CONNECT")     block.connectionType = ConnectionType::X_CONNECT;

			std::string diffusePath = "assets/textures/block/diffuse/";
			std::string normalPath = "assets/textures/block/normal/";

			if (blockState["allTextures"].valid()) {
				std::string path = blockState["allTextures"];
				block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(diffusePath + path);
				block.texture[(int)BlockTexture::BOTTOM] = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::FRONT]  = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::BACK]   = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::LEFT]   = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::RIGHT]  = block.texture[(int)BlockTexture::TOP];

				block.normalTexture[(int)BlockTexture::TOP] = assets.LoadNormalTexture(diffusePath + path);
				block.normalTexture[(int)BlockTexture::BOTTOM] = block.normalTexture[(int)BlockTexture::TOP];
				block.normalTexture[(int)BlockTexture::FRONT] =  block.normalTexture[(int)BlockTexture::TOP];
				block.normalTexture[(int)BlockTexture::BACK] =   block.normalTexture[(int)BlockTexture::TOP];
				block.normalTexture[(int)BlockTexture::LEFT] =   block.normalTexture[(int)BlockTexture::TOP];
				block.normalTexture[(int)BlockTexture::RIGHT] =  block.normalTexture[(int)BlockTexture::TOP];

				load((diffusePath + path).c_str(), items[currentTexture].texture);
			}
			else {
				std::string itemPath;
				std::string path;
				if (blockState["top"].valid()) {
					itemPath = blockState["top"];
					path = blockState["top"]; block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(diffusePath + path);
					block.normalTexture[(int)BlockTexture::TOP] = assets.LoadNormalTexture(normalPath + path);
				}
				if (blockState["bottom"].valid()) {
					path = blockState["bottom"]; block.texture[(int)BlockTexture::BOTTOM] = assets.LoadTexture(diffusePath + path);
					block.normalTexture[(int)BlockTexture::BOTTOM] = assets.LoadNormalTexture(normalPath + path);
				}
				if (blockState["front"].valid())  { 
					path = blockState["front"];  block.texture[(int)BlockTexture::FRONT]  = assets.LoadTexture(diffusePath + path);  
					block.normalTexture[(int)BlockTexture::FRONT] = assets.LoadNormalTexture(normalPath + path);
				}
				if (blockState["back"].valid())   { 
					path = blockState["back"];   block.texture[(int)BlockTexture::BACK]   = assets.LoadTexture(diffusePath + path);
					block.normalTexture[(int)BlockTexture::BACK] = assets.LoadNormalTexture(normalPath + path);
				}
				if (blockState["left"].valid())   { 
					path = blockState["left"];   block.texture[(int)BlockTexture::LEFT]   = assets.LoadTexture(diffusePath + path);  
					block.normalTexture[(int)BlockTexture::LEFT] = assets.LoadNormalTexture(normalPath + path);
				}
				if (blockState["right"].valid())  {
					path = blockState["right"];  block.texture[(int)BlockTexture::RIGHT]  = assets.LoadTexture(diffusePath + path); 
					block.normalTexture[(int)BlockTexture::RIGHT] = assets.LoadNormalTexture(normalPath + path);
				}

				itemPath = diffusePath + itemPath;
				load(itemPath.c_str(), items[currentTexture].texture);
			}
			
			items[currentTexture].block = blockID;
			currentTexture++;

			if (blockState["emitLight"].valid()) block.emitLight = blockState["emitLight"];
		}
		static void AddBiome(const Biome biome) {			
			biomeMap[currentBiomeID] = biome;
			currentBiomeID++;
		}

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

	public:
		// Multiplayer
		bool multiPlayer : 1;
		bool bWaitingForConnection = true;
		net::client_interface<GameMsg> clientInstance;

		Player p;
		Font font;
#define NUM_LIGHTS chunkVolume
		
		struct Light {
			uint32_t color;
			alignas(16) glm::vec3 vector;
		} lighting[NUM_LIGHTS];

		void Create() {
			chunkShader.Create("shaderpacks/default/chunkShader.glsl");

			transforms.Create(nullptr, sizeof(TransformData), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			transforms.BufferBase(0);

			lights.Create(nullptr, sizeof(lighting), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			lights.BufferBase(1);

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

			//Loading blocks
			worldGenState.set_function("AddBlockScript", &Singleplayer::AddBlockScript);
			worldGenState.script_file("scripts/blocks.lua");

			worldGenState.new_usertype<Biome>("Biome", sol::constructors<void()>(),
				"maxMois", &Biome::maxMois,
				"maxTemp", &Biome::maxTemp,
				"minMois", &Biome::minMois,
				"minTemp", &Biome::minTemp,
				"topBlock", &Biome::topBlock
				);
			worldGenState.set_function("AddBiome", &Singleplayer::AddBiome);
			worldGenState.script_file("scripts/biomes.lua");
			assets.Free();

			load("assets/textures/misc/cursor.png", assets.textures[0]);
			load("assets/textures/misc/hotbar.png", assets.textures[1]);
			load("assets/textures/misc/hotbar_selected.png", assets.textures[2]);
			load("assets/textures/misc/hearts.png", assets.textures[3]);

			lineBatcher.Create();
			chunkMeshArray.Create();
			chunkMeshArray.VertexAttribPointer(0, 3, offsetof(Vertex, Position));  // position attribute
			chunkMeshArray.VertexAttribPointer(1, 1, offsetof(Vertex, TexCoords)); // texture coord attribute
			chunkMeshArray.VertexAttribPointer(2, 3, offsetof(Vertex, Normal)); // type attribute
			chunkMeshArray.VertexAttribPointer(3, 1, offsetof(Vertex, color)); // color attribute
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				//Configuring the vertex buffer
				chunks[chunkID].meshBuffer.Create(nullptr, MaxVertexCount * sizeof(Vertex), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
				chunks[chunkID].indexBuffer.Create(nullptr, sizeof(uint32_t) * MaxFaceCount * 6, GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			}

			skybox.Create("shaderpacks/default/skybox.glsl");
#ifdef MODEL
			modelShader.Create("shaderpacks/default/modelShader.glsl");
			model.Create("assets/models/deccer cubes/SM_Deccer_Cubes_Textured.obj");
			//animation.Create("assets/models/dancing_vampire.dae", model);
#endif // MODEL
			X_FACE1.CalculateNormal();
			X_FACE2.CalculateNormal();			

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

			LoadWorld("New world");
		}

		void Connect(const std::string& ip, const std::string& playerName) {
			clientInstance.Connect(ip, 60000); // 25.32.4.119
			p.name = playerName;
		}

		void CreateNewWorld(const std::string& name) {
			worldName = name;
			std::filesystem::create_directories("worlds/" + worldName +"/Chunk data/Island 0");
			std::filesystem::create_directories("worlds/" + worldName + "/Entity data");
			SaveWorld();
			SavePlayerState(p);
		}

		void LoadWorld(const std::string& name) {
			worldName = name;
			p.Position = { 0, (RenderDistance * RenderDistance * 0.5f), 0 };
			LoadPlayerState(p, p.name);
			glm::ivec3 offset = getChunkPos(p.Position) - glm::ivec3(chunkSize / 2);
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				chunks[chunkID].position = to3D(chunkID, glm::ivec3(RenderDistance)) + offset;

				UpdateNeighbours(chunkID);
				TryToLoadChunk(chunkID);
			}

			YAML::Node config = YAML::LoadFile("worlds/" + worldName + "/world.properties");
			if (config["time"]) skybox.angle = config["time"].as<float>();
			if (config["seed"]) worldNoise.SetSeed(config["seed"].as<int>());
		}

		void SaveWorld() {
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) SaveChunk(chunkID);

			YAML::Node config;
			config["seed"] = worldNoise.GetSeed();
			config["time"] = skybox.angle;
			YAMLUtils::saveFile("worlds/" + worldName + "/world.properties", config);

			SavePlayerState(p);
		}
		
		void Update(const float& deltaTime) {
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
							WC_INFO("Server accepted client - you're in!");
							net::message<GameMsg> msg;
							msg.header.id = GameMsg::Client_RegisterWithServer;
							descPlayer.Position = glm::vec3(0.f);
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
							WC_INFO("New Player Joined");
							if (desc.nUniqueID == localPlayerID)
							{
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
							glm::ivec4 sblockData = glm::ivec4(0);
							msg >> sblockData;
							glm::ivec3 pos = { sblockData.x, sblockData.y, sblockData.z };
							setBlock(pos, sblockData.w, true, false);
							break;
						}
						}
					}
				}

				if (bWaitingForConnection) WC_INFO("Waiting for connection");// Tell the client you are waiting

				net::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_UpdatePlayer;
				players[localPlayerID].Position = p.Position;
				msg << players[localPlayerID];
				clientInstance.Send(msg);

				for (auto& player : players) {
					if (player.second.nUniqueID != localPlayerID) {
						DrawOutlineCube(player.second.Position - p.Size, p.Size * 2.f, glm::vec4(1.f));
					}
				}
			}

			// camera/view transformation
			TransformData data;
			data.windowSize = window.GetSize();
			data.proj = glm::perspective(glm::radians(camera.FOV), data.windowSize.x / data.windowSize.y, 0.1f, 1100.f);
			data.view = camera.GetViewMatrix();
			data.dt = deltaTime;
			data.cameraPos = camera.Position;
			data.lower_left_corner = camera.lower_left_corner;
			data.vertical = camera.vertical;
			data.horizontal = camera.horizontal;
			data.numLights = currentLightID;

			lighting[0].vector = -glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * glm::rotate(glm::mat4(1.f), glm::radians(skybox.angle), glm::vec3(0.f, 0.f, 1.f)));
			//lighting[0].color = convertColor(glm::vec4(glm::vec3(glm::dot(lighting[0].vector, glm::vec3(0.f, 1.f, 0.f))), 0.f));
			transforms.SetData(0, sizeof(TransformData), &data);
			if (lightUpdate) {
				lights.SetData(0, sizeof(Light) * currentLightID, lighting);
				lightUpdate = false;
			}
			else 
				lights.SetData(0, sizeof(Light), lighting);			

			glDisable(GL_DEPTH_TEST);
			skybox.Draw(deltaTime);
			glEnable(GL_DEPTH_TEST);

			viewFrustum.update(data.proj * data.view);
			uint32_t chunkHalf = chunkSize / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position);

			for (ChunkID i = 0; i < chunks.size(); i++) {
				glm::vec3 currChunkPos = chunks[i].position;
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

			assets.Bind(0);
			//assets.BindNormal(1);
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
				DrawOutlineCube(p.Position - p.Size, p.Size * 2.f, glm::vec4(1.f));
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
			// GUI
			glDisable(GL_DEPTH_TEST);
			const float scale = 0.35f;			

			//Inventory
			const float hotbarSize = 48.f;
			float oneSixth = hotbarSize / 6.f;
			glm::vec2 hotbarStart = glm::vec2((data.windowSize.x - inventorySizeX * hotbarSize) * 0.5f, data.windowSize.y - hotbarSize); // Temp until inventory

			//Health
			glm::vec2 offset = glm::vec2(0.f);
			glm::vec2 healthSize = glm::vec2(20.f);
			glm::vec2 healthStart = glm::vec2(hotbarStart.x, data.windowSize.y - hotbarSize - 2.f - healthSize.y) - (glm::vec2(3.f, -healthSize.x) + healthSize);

			for (uint8_t i = 0; i < (int8_t)p.health; i++) {
				offset += glm::vec2(3.f, -healthSize.x) + healthSize;
				Renderer2D::DrawQuad(healthStart + offset, healthSize, assets.textures[3], { 0,0 }, { 7, 7 });
			}
			
			if ((float)((int)p.health) < p.health) 
			Renderer2D::DrawQuad(healthStart + offset + healthSize + glm::vec2(3.f, -healthSize.x), { healthSize.x * 0.5f + 2.f, healthSize.y }, assets.textures[3], { 0,0 }, { 4, 7 });
			
			console.start = { 25.f, 0.f };
			console.DrawTextLine("FPS: " + std::to_string((int)(1.f / deltaTime)) + " Frametime: " + std::to_string(deltaTime * 1000), font);
			console.DrawTextLine("X: " + std::to_string(p.Position.x) + " Y: " + std::to_string(p.Position.y) + " Z: " + std::to_string(p.Position.z), font);
			console.DrawTextLine("Pitch: " + std::to_string(p.rotation.x) + " Yaw: " + std::to_string(p.rotation.y) + " Roll: " + std::to_string(camera.Roll), font);
			console.DrawTextLine(
				"ChunkX: " + std::to_string(currentPlayerPos.x) +
				" ChunkY: " + std::to_string(currentPlayerPos.y) +
				" ChunkZ: " + std::to_string(currentPlayerPos.z), font);
			float VelY = p.velocity.y;
			if (p.flying) VelY /= 9.f;
			console.DrawTextLine("Velocity: X: " + std::to_string(p.velocity.x / 9.f) +
								 " Y: " + std::to_string(VelY) + 
								 " Z: " + std::to_string(p.velocity.z / 9.f), font);
			console.DrawTextLine("Time of the day: " + std::to_string(skybox.angle / 6.f * 144.f), font);
			//Renderer2D::DrawText("Heap Memory: " + std::to_string(modelScale) + " bytes", font, { 25.f, 65.f * scale * 10.f}, scale);
			console.DrawTextLine("Number of lights: " + std::to_string(currentLightID), font);
			console.DrawTextLine("Look at: X: " 
				+ std::to_string((int)floor(m_rayEnd.x)) + " Y: " 
				+ std::to_string((int)floor(m_rayEnd.y)) + " Z: " 
				+ std::to_string((int)floor(m_rayEnd.z)) + " Looking at block: " 
				+ std::to_string(getBlock(m_rayEnd)), font);

			console.line += 10;
			console.DrawTextLine(textbox.text, font);

			console.Reset();
			
			for (uint8_t i = 0; i < inventorySizeX; i++) {
				uint8_t id = 0;
				if (p.currentSlot == i) id = 1;
				Renderer2D::DrawQuad(hotbarStart + glm::vec2(hotbarSize * i, 0.f), { hotbarSize,hotbarSize }, assets.textures[1 + id]);
			
				uint32_t amount = p.inventory.data[i].stack_size;
				if (amount > 0) 
					Renderer2D::DrawQuad(hotbarStart + glm::vec2(hotbarSize * i + 4.f, 4.f), { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[p.inventory.data[i].itemID].texture);
				if (amount > 1)
					Renderer2D::DrawText(std::to_string(amount), font, hotbarStart + glm::vec2(hotbarSize * i + 4.f, 44.f), 0.4f, glm::vec4(1.f));
			}
			glm::vec2 cursorSize = (glm::vec2)assets.textures[0].GetSize() * 1.4f;
			glm::vec2 cursorPos = glm::vec2(data.windowSize.x - cursorSize.x, data.windowSize.y - cursorSize.y) * 0.5f;
			Renderer2D::DrawQuad(cursorPos, cursorSize, assets.textures[0]);
			glEnable(GL_DEPTH_TEST);
		}

		glm::vec3 m_rayEnd;
		glm::vec3 m_rayStart;
		bool thirdPerson = false;

		void OnInput(bool& HasFocus, const float& deltaTime) {
			glm::ivec2 windSize = window.GetSize();
			// MENU MANAGMENT
			if (Keyboard::getKey(Keyboard::Key::E) == GLFW_PRESS && !textbox.isSelected) mode = MenuMode::INVENTORY;

			if (Keyboard::getKey(Keyboard::Key::Escape) == GLFW_PRESS) mode = MenuMode::ESCMENU;

			if (wc::bReloadChunkShader) { 
				chunkShader.Destroy(); 
				chunkShader.Create("shaderpacks/default/chunkShader.glsl");
				wc::bReloadChunkShader = false;
			}

			if (wc::bReloadModelShader) {
#ifdef MODEL
				modelShader.Destroy();
				modelShader.Create("shaderpacks/default/modelShader.glsl");
#endif // MODEL
				wc::bReloadModelShader = false;
			}

			if (Keyboard::getKey(Keyboard::Key::Enter) && !textbox.isSelected) 
				textbox.isSelected = true;			
			else if (Keyboard::getKey(Keyboard::Key::Enter) && textbox.isSelected) {
				// Command parsing
				textbox.isSelected = false;
				std::string args;
				CommandType commandType = getCommandType(textbox.text, args);
				args += ' ';
				if (commandType == CommandType::textMessage) WC_INFO(textbox.text);
				else if (commandType == CommandType::fly) {
					p.flying = getArgument(args);
					p.wasOnGround = false;
					p.m_isOnGround = false;
					p.wasFalling = false;
				}
				else if (commandType == CommandType::collide) p.collision = getArgument(args);
				else if (commandType == CommandType::setBlock) setBlock({ getArgument(args, 1) , getArgument(args, 2) , getArgument(args, 3) }, getArgument(args), true, true);
				else if (commandType == CommandType::give) p.inventory.AddItem(getArgument(args, 0), 0, getArgument(args, 1));
				else if (commandType == CommandType::setSpeed) p.MovementSpeed = getArgument(args, 0);
				else if (commandType == CommandType::setTime) skybox.angle = getArgument(args, 0);
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
				textbox.text = "";
			}
			textbox.update();

			if (!textbox.isSelected) {

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

				if (mouseScrolled) {
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
			
			Mouse::ShowMouse(!HasFocus);

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
						DrawOutlineCube(pos, glm::vec3(1.f), glm::vec4(1.f));
						if (bBreak) {
							p.inventory.AddItem(block - 1, p.currentSlot);
							setBlock(pos, 0, true, true);
						}
						else if (bPlace) {
							ItemID itemID = p.inventory.data[p.currentSlot].itemID;
							if (p.inventory.RemoveItem(p.currentSlot))
								setBlock(m_rayLastPos, items[itemID].block, true, true);
						}
						break;
					}
				}
				m_rayLastPos = pos;
			}
		}

		void Destroy() {
			SaveWorld();
		}

	private:

		void collide(const glm::vec3& vel) {
			glm::vec3 startPos = p.Position - p.Size;
			glm::vec3 endPos = p.Position + p.Size;

			if (startPos.x > endPos.x) std::swap(startPos.x, endPos.x);
			if (startPos.y > endPos.y) std::swap(startPos.y, endPos.y);
			if (startPos.z > endPos.z) std::swap(startPos.z, endPos.z);

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
		void TryToLoadChunk(const ChunkID& chunkID) {
			//if (multiPlayer && clientInstance.IsConnected()) {
			//	net::message<GameMsg> msg;
			//	msg.header.id = GameMsg::RequestChunk;
			//	glm::ivec3 position = chunks[chunkID].position;
			//	msg << position;
			//	clientInstance.Send(msg);
			//}
			//else {
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
			//}
		}

		void SaveChunk(const ChunkID& chunkID) {
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
								if (pos.y == 125) setBlockLocal(glm::ivec3(x, y, z), 1, chunk, false, false);
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
			for (uint32_t i = 1; i < currentBiomeID; i++) {
				if (temperature >= biomeMap[i].minTemp && temperature <= biomeMap[i].maxTemp
					//&& moisture >= biomeMap[i].minMois && moisture <= biomeMap[i].maxMois
					) return i;
			}
			return 0;
		}

		// LIGHT MANAGING
		uint32_t addLight(const glm::vec3& position, const uint32_t& color) {
			uint32_t light = currentLightID;
			if (currentLightID <= NUM_LIGHTS) {
				lighting[currentLightID].vector = position;
				lighting[currentLightID].color = color;
				currentLightID++;
				lightUpdate = true;
			}
			return light;
		}

		void removeLight(const glm::vec3& position) {
			for (uint32_t i = 0u; i < NUM_LIGHTS; i++)
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
			if (playerEdited) chunks[chunk].used = true;

			if (replyToServer && multiPlayer) {
				net::message<GameMsg> msg;
				msg.header.id = GameMsg::BlockEdit;
				glm::ivec4 test = glm::ivec4(pos + chunks[chunk].position * glm::ivec3(chunkSize), block);
				msg << test;
				clientInstance.Send(msg);
			}

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

			auto addMesh = [&](const Face& face, const glm::vec3& pos, const BlockID& block, const glm::vec2& size = glm::vec2(1.f, 1.f)) {
				if (chunk.IndexCount >= MaxFaceCount * 6) { /*WC_ERROR("Memory overflow!");*/ return; }

				glm::vec2 TexCoords[] = {
					glm::vec2(0.f,    0.f),
					glm::vec2(0.f,    size.y),
					glm::vec2(size.x, size.y),
					glm::vec2(size.x, 0.f),
				};
				
				for (uint32_t i = 0; i < ARRAYSIZE(face.corner); i++) 
					vertices[i + offset] = Vertex(face.corner[i] + pos, { TexCoords[i], face.texID }, blockData[block].connectionType, blockData[block].color, face.normal);
				
				for (uint32_t i = 0; i < ARRAYSIZE(indexArray); i++)
					indices[chunk.IndexCount + i] = indexArray[i] + ioffset;

				ioffset += ARRAYSIZE(face.corner);
				chunk.IndexCount += 6;
				offset += 4;		
			};

			bool done = false;
			uint32_t i = 0, j = 0, k = 0, l = 0, w = 0, h = 0, d = 0, u = 0, v = 0, n = 0;
			uint8_t type = ConnectionType::NON_EXISTENT, checkType = ConnectionType::NON_EXISTENT;
			BlockID mask[chunkSize * chunkSize];
			uint32_t textureMask[chunkSize * chunkSize];
			BlockID block = 0, checkBlock = 0;
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
							block = 0;
							type = ConnectionType::NON_EXISTENT;
							checkBlock = 0;
							checkType = ConnectionType::NON_EXISTENT;

							if (x[d] >= 0) {
								block = chunk.data[x[0]][x[1]][x[2]];
								type = blockData[block].connectionType;
							}
							//else if (chunks[chunk].neighborNeg[d] > -1) {
							//	int x1 = x[d];
							//	x[d] = chunkSize - 1;
							//	block = chunks[chunks[chunk].neighborNeg[d]].data[x[0]][x[1]][x[2]];
							//	type = blockData[block].connectionType;
							//	x[d] = x1;
							//}

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

							if (type == ConnectionType::X_CONNECT) {
								Face face1 = X_FACE1;
								Face face2 = X_FACE2;
								face1.texID = blockData[block].texture[(int)BlockTexture::TOP];
								face2.texID = blockData[block].texture[(int)BlockTexture::TOP];
								addMesh(face1, chunkPos + x, block);
								addMesh(face2, chunkPos + x, block);
							}
							//else if (type == ConnectionType::CUSTOM_MODEL) {
							//	addMesh(BlockMeshes, chunkPos + x, block);
							//}
							
							if (type != checkType && type != ConnectionType::NON_EXISTENT && checkType != ConnectionType::NON_EXISTENT) {
								if (block != checkBlock && type == ConnectionType::CONNECT_DEFAULT) { mask[n] = block + 1; textureMask[n] = blockData[block].texture[d]; }
								else if ((block == 0 || type != ConnectionType::CONNECT_DEFAULT) && checkType == ConnectionType::CONNECT_DEFAULT) { mask[n] = checkBlock + 1; textureMask[n] = blockData[checkBlock].texture[d + 3]; }

								else if (block != checkBlock && type == ConnectionType::NO_CONNECT) { mask[n] = block + 1; textureMask[n] = blockData[block].texture[d]; }
								else if (block == 0 && checkType == ConnectionType::NO_CONNECT)		{ mask[n] = checkBlock + 1; textureMask[n] = blockData[checkBlock].texture[d + 3]; }

								else if (checkBlock == 0 && type == ConnectionType::FLUID_CONNECT)  { mask[n] = block + 1; textureMask[n] = blockData[block].texture[d]; }
								else if (block == 0 && checkType == ConnectionType::FLUID_CONNECT)  { mask[n] = checkBlock + 1; textureMask[n] = blockData[checkBlock].texture[d + 3]; }
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
								face.texID = textureMask[n];
								
								addMesh(face, chunkPos, mask[n] - 1, glm::vec2(h1, w1));
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

		// UTILS
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
	};	
}