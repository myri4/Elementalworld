#pragma once
#define DISABLE_CACHING 1

#include <pch.h>
#include "../Rendering/AssetManager.h"
#include "Chunk.h"
#include "Biome.h"
#include "Player.h"
#include "CommandParser.h"
#include <FastNoise/FastNoiseLite.h>

#include "../Rendering/Renderer3D.h"
#include <stb_image/stb_write.h>
#include "../Settings.h"

namespace wc {	

	// Raw game data
	inline List<Block, 40> m_BlockData;
	inline List<Biome, 40> m_BiomeData;

	class GameInstance {

		// Player related
		float MouseSensitivity = 5.f;
		float gravity = 20.f;

		Timer m_BlockBreakTimer;
		bool startBreaking = false;
		glm::ivec3 breakPos = glm::ivec3(0);
		bool thirdPerson = false;

		// Graphics
		Renderer3D m_Renderer;
		Camera& camera = m_Renderer.camera;
		Texture crosshair;

		float rotateSpeed = 1.f * 0.6f; // one cycle is one unit (in minutes)
		float angle = 0.f;


		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> chunks;

		FastNoiseLite worldNoise;
		FastNoiseLite treeNoise;

		// Flags
		bool generateTerrain = false;
		bool updateChunks = true;
		bool updateOctree = true;

		bool console = false;
		bool inventory = false;
		bool renderGUI = true;
		bool debug_menu = true;

		int8_t water_level = 32;

		// Common blocks
		BlockID grass = 0;
		BlockID stone = 0;
		BlockID water = 0;
		BlockID sand = 0;
		BlockID oak = 0;
		BlockID leaves = 0;
		BlockID coal = 0;
		BlockID dirt = 0;
		BlockID campfire = 0;
		BlockID murshroom = 0;

		BlockID blockHolding = 0;

		uint32_t localPlayerID = 0;
		std::unordered_map<uint32_t, PlayerDescription> players;

		// Data managing
		bool m_WorldLoaded = false;
		// Multiplayer
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

		bool m_WaitingForConnection = true;
		net::client_interface<GameMsg> m_ClientInstance;

		// Command console
		std::vector<std::string> consoleHistory;
		char consoleBuffer[256];
	public:
		std::string worldName = "New world";
		bool multiPlayer = false;
		Player p;

		void Create(AssetManager& assetManager) {
			assetManager.Init();
			crosshair = assetManager.LoadImage(GetAssetPath() + "/textures/misc/cursor.png");

			if (!std::filesystem::exists("worlds")) std::filesystem::create_directory("worlds");
			if (!std::filesystem::exists("cache")) std::filesystem::create_directory("cache");
			if (!std::filesystem::exists("settings.yaml")) Settings::Save();
			if (!std::filesystem::exists("servers.yaml")) {
				YAML::Node servers;
				YAMLUtils::saveFile("servers.yaml", servers);
			}

			Settings::Load();

			worldNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
			worldNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
			worldNoise.SetFractalOctaves(9);
			worldNoise.SetMultiplier(256.f);
			worldNoise.SetFrequency(1.f / 2000.f);//scale
			worldNoise.SetFractalLacunarity(2.f);
			worldNoise.SetFractalGain(0.53f);//persistance, roughness

			treeNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
			treeNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_Ridged);
			treeNoise.SetFractalOctaves(5);
			treeNoise.SetFractalLacunarity(1.f);
			treeNoise.SetFrequency(1.f / 3.f);
			treeNoise.SetFractalGain(0.003f);

			
			wc::StagingBuffer matBuffer;
			matBuffer.Create(materialData.byte_size());
			
			m_BlockData.counter = 1;
			materialData.Data = (BufferMaterial*)matBuffer.Map();
			materialData.counter = 1;
			itemData.counter = 1;
			
			std::string diffusePath = GetAssetPath() + "/textures/block/diffuse/";
			std::string materialPath = GetAssetPath() + "/textures/block/materials/";
			
			//Loading blocks
			std::vector<Renderer3D::Vertex> modelVertices;
			std::vector<uint32_t> modelIndices;
			std::vector<Node> bvhData;

			auto setMaterial = [&](const YAML::Node& materialScript, BufferMaterial& material) {
				if (materialScript["albedo"])
					material.albedo = assetManager.LoadTexture(diffusePath + materialScript["albedo"].as<std::string>());
				else {
					//WC_ERROR("Albedo map is not defined for {}! Skipping this block", script);
					//continue;
				}

				if (materialScript["reflectance"])
					material.reflectance = materialScript["reflectance"].as<float>();
				else if (materialScript["reflectancePath"])
					material.reflectance = -(float)assetManager.LoadTexture(materialPath + materialScript["reflectancePath"].as<std::string>());


				if (materialScript["metallic"])
					material.metallic = materialScript["metallic"].as<float>();
				else if (materialScript["metallicPath"])
					material.metallic = -(float)assetManager.LoadTexture(materialPath + materialScript["metallicPath"].as<std::string>());


				if (materialScript["roughness"])
					material.roughness = materialScript["roughness"].as<float>();
				else if (materialScript["roughnessPath"])
					material.roughness = -(float)assetManager.LoadTexture(materialPath + materialScript["roughnessPath"].as<std::string>());


				if (materialScript["emissive"])
					material.emissive = materialScript["emissive"].as<float>();
				else if (materialScript["emissivePath"])
					material.emissive = -(float)assetManager.LoadTexture(materialPath + materialScript["emissivePath"].as<std::string>());


				if (materialScript["clearCoat"])
					material.clearCoat = materialScript["clearCoat"].as<float>();
				else if (materialScript["clearCoatPath"])
					material.clearCoat = -(float)assetManager.LoadTexture(materialPath + materialScript["clearCoatPath"].as<std::string>());
				
				
				if (materialScript["anisotropy"])
					material.anisotropy = materialScript["anisotropy"].as<float>();
				else if (materialScript["anisotropyPath"])
					material.anisotropy = -(float)assetManager.LoadTexture(materialPath + materialScript["anisotropyPath"].as<std::string>());
				
				//// @TODO: Should be rotation instead
				//if (materialScript["anisotropyDirection"])
				//	material.anisotropyDirection = materialScript["anisotropyDirection"].as<glm::vec3>();
				
				if (materialScript["ior"])
					material.ior = materialScript["ior"].as<float>();
				else if (materialScript["iorPath"])
					material.ior = -(float)assetManager.LoadTexture(materialPath + materialScript["iorPath"].as<std::string>());
			};

			for (auto& path : std::filesystem::directory_iterator("scripts/blocks")) {
				std::string filename = path.path().stem().string();
				if (path.is_regular_file()) { //AddBlockScript
					std::string script = "scripts/blocks/" + filename + ".yaml";
					YAML::Node blockState = YAML::LoadFile(script);
			
					Block block;
					BufferMaterial materials[6];
			
					if (blockState["name"]) block.name = blockState["name"].as<std::string>();					
					else WC_WARN("No block name is specified in '{}'. Block name 'air' assumed.", script);
			
					if (blockState["isCollidable"]) block.isCollidable = blockState["isCollidable"].as<bool>();
					if (blockState["ConnectionType"]) block.connectionType = magic_enum::enum_cast<ConnectionType>(blockState["ConnectionType"].as<std::string>()).value();

					//if (blockState["cull"]) if (blockState["cull"].as<bool>()) material.flags |= WC_CULL_BIT;
			
					if (blockState["modelPath"]) { 
						block.flags |= WC_MODEL_BIT;
						block.connectionType = ConnectionType::CUSTOM_MODEL;
					}

					if (blockState["allTextures"]) {
						setMaterial(blockState["allTextures"], materials[0]);

						uint32_t materialID = materialData.push_back(materials[0]);
						for (int i = 0; i < 6; i++) block.materialIDs[i] = materialID;
					}
					else {
						for (uint32_t i = 0; i < magic_enum::enum_count<BlockTexture>(); i++) {
							auto name = std::string(magic_enum::enum_name((BlockTexture)i));
							setMaterial(blockState[name], materials[i]);
							block.materialIDs[i] = materialData.push_back(materials[i]);
						}
					}
					

					if (blockState["modelPath"]) {
						std::string path = blockState["modelPath"].as<std::string>();
						block.meshID = blockMeshes.size();
						blockMeshes[block.meshID].Load(GetAssetPath() + "/models/" + path, block.materialIDs[0], modelVertices, modelIndices, bvhData);
						blockMeshes.counter++;
					}
			
					m_BlockData.PushBack(block);
				}
			}

			for (auto& path : std::filesystem::directory_iterator("scripts/items")) {
				std::string filename = path.path().stem().string();
				if (path.is_regular_file()) { 
					std::string script = "scripts/items/" + filename + ".yaml";
					YAML::Node itemState = YAML::LoadFile(script);
					Item item;
					if (itemState["displayName"]) item.displayName = itemState["displayName"].as<std::string>();
					else item.displayName = "No name is set";

					if (itemState["name"]) item.name = itemState["name"].as<std::string>();
					else item.name = "unnamed_item";

					if (itemState["maxStackSize"]) item.maxStackSize = itemState["maxStackSize"].as<uint32_t>();
					if (itemState["maxDurability"]) item.maxDurability = itemState["maxDurability"].as<uint32_t>(); // automatically flag that this item should have durability

					if (itemState["block"]) { item.block = getBlockID(itemState["block"].as<std::string>()); }

					if (itemState["type"]) { }

					if (itemState["model"]) {}

					if (itemState["textureLocation"]) item.textureID = assetManager.LoadTexture(diffusePath + itemState["textureLocation"].as<std::string>());

					itemData.PushBack(item);
				}
			}
			Renderer3D::Init(assetManager);
			Renderer3D::CreateBuffers(m_BlockData.size(), chunks.size());
			m_Renderer.Create(window.GetSize(), assetManager);
			wc::BlockGPU gpuBlockData[m_BlockData.allocatedSize()];

			for (int i = 0; i < m_BlockData.size(); i++)
				for (int j = 0; j < 6; j++) 
					gpuBlockData[i].materialIDs[j] = m_BlockData[i].materialIDs[j];
			

			m_Renderer.GetBlockDataBuffer().SetData(gpuBlockData, sizeof(BlockGPU) * m_BlockData.size());

			matBuffer.Unmap();
			m_Renderer.GetMaterialsBuffer().SetData(matBuffer, materialData.byte_size());
			matBuffer.Destroy();

			Renderer3D::UploadModels(modelVertices, modelIndices, bvhData);	
			
			m_Renderer.addLight(glm::vec3(0.f), glm::vec3(1.f, 0.891f, 0.796f), 1.f, 0.f);
			
			grass = getBlockID("grass_block");
			stone = getBlockID("stone_block");
			coal = getBlockID("coal_ore");
			water = getBlockID("water");
			sand = getBlockID("sand");
			oak = getBlockID("wood");
			leaves = getBlockID("leaves");
			dirt = getBlockID("dirt");
			campfire = getBlockID("campfire");
			murshroom = getBlockID("murshroom");
			blockHolding = getBlockID("iron_block");
		}

		void Destroy() {
			if (multiPlayer) m_ClientInstance.Disconnect();
			else if (m_WorldLoaded) SaveWorld();			

			m_Renderer.Destroy();
			Renderer3D::Deinit();			

			Settings::Save();
		}

		void Join(const std::string& ip, const std::string& playerName) {
			if (multiPlayer) {
				m_ClientInstance.Connect(ip, 60000);
				p.name = playerName;
			}
			LoadWorld();
		}		

		void Update(float deltaTime) {			
			// Multiplayer 
			if (multiPlayer) {
				if (m_ClientInstance.IsConnected())
				{
					while (!m_ClientInstance.Incoming().empty())
					{
						auto msg = m_ClientInstance.Incoming().pop_front().msg;

						switch (msg.header.id)
						{
						case(GameMsg::Client_Accepted):
						{
							net::message<GameMsg> server_msg;
							server_msg.header.id = GameMsg::Client_RegisterWithServer;
							PlayerDescription descPlayer;
							server_msg << descPlayer;
							m_ClientInstance.Send(server_msg);
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
								m_WaitingForConnection = false;
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
							generateTerrain = true;
							break;
						}
						}
					}
					for (auto& player : players) {
						if (player.second.nUniqueID != localPlayerID) {
							//Renderer3D::DrawOutlineCube(player.second.Position - p.Size, p.Size * 2.f, glm::vec4(1.f));
						}
					}
				}

				if (m_WaitingForConnection) WC_INFO("Waiting for connection");// Tell the client you are waiting

				net::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_UpdatePlayer;
				players[localPlayerID].Position = p.Position;
				players[localPlayerID].currentSlot = p.currentSlot;
				players[localPlayerID].health = p.health;
				players[localPlayerID].name = p.name;
				players[localPlayerID].rotation = p.rotation;
				msg << players[localPlayerID];
				m_ClientInstance.Send(msg);
			}

			m_Renderer.m_Lights[0].vector = -glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)));

			angle += deltaTime * rotateSpeed;
			angle = glm::mod(angle, 360.f);

			uint32_t chunkHalf = chunkSize / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position); // @TODO: hmmm? why doesnt it work with glm::ivec3?
			for (ChunkID i = 0; i < chunks.size(); i++) {
				glm::ivec3& currChunkPos = chunks[i].position;
				glm::ivec3 newPosition = currChunkPos;
				//if (currChunkPos.x < currentPlayerPos.x - chunkHalf) newPosition.x = currentPlayerPos.x + chunkHalf - 1;
				//else if (currChunkPos.x > currentPlayerPos.x + chunkHalf) newPosition.x = currentPlayerPos.x - chunkHalf + 1;
				//
				//else if (currChunkPos.y < currentPlayerPos.y - chunkHalf) newPosition.y = currentPlayerPos.y + chunkHalf - 1;
				//else if (currChunkPos.y > currentPlayerPos.y + chunkHalf) newPosition.y = currentPlayerPos.y - chunkHalf + 1;
				//
				//else if (currChunkPos.z < currentPlayerPos.z - chunkHalf) newPosition.z = currentPlayerPos.z + chunkHalf - 1;
				//else if (currChunkPos.z > currentPlayerPos.z + chunkHalf) newPosition.z = currentPlayerPos.z - chunkHalf + 1;
			
				if (newPosition != currChunkPos) // Reset Chunk
				{
					SaveChunk(i);
					chunks[i].position = newPosition;
					TryToLoadChunk(i);		

					chunks[i].canBeUpdated = true;
					updateChunks = true;
					updateOctree = true;
				}
			}
			
			if (generateTerrain) {
				for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
					if (!chunks[chunk].generated) { GenerateChunkTerrain(chunk); chunks[chunk].generated = true; }
			
				//for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
				//	if (!chunks[chunk].generatedStructures) { GenerateChunkStructures(chunk); chunks[chunk].generatedStructures = true; }
			
				generateTerrain = false;
			}

			if (updateChunks) {
				updateChunks = false;

				for (ChunkID i = 0; i < chunks.size(); i++)
					if (chunks[i].canBeUpdated) { 
						UpdateChunk(i);				
						
						chunks[i].canBeUpdated = false; 
					}

				if (updateOctree) {
					updateOctree = false;

					std::vector<ChunkNode> nodes;
					ChunkNode rootNode;


					glm::vec3& start = rootNode.min = glm::vec3(0.f);
					glm::vec3& end = rootNode.max = glm::vec3(0.f);
					for (ChunkID i = 0; i < chunks.size(); i++) {
						auto chunkStart = chunks[i].position * glm::ivec3(chunkSize);
						auto chunkEnd = chunkStart + glm::ivec3(chunkSize);

						for (int i = 0; i < 3; i++) {
							if (chunkStart[i] < start[i]) start[i] = chunkStart[i];
							else if (chunkEnd[i] > end[i]) end[i] = chunkEnd[i];
						}
					}
					nodes.push_back(rootNode);
					GenerateOctree(nodes, 0, 4);

					m_Renderer.GetChunkNodeBuffer().SetData(nodes.data(), sizeof(ChunkNode)* nodes.size());
				}
			}

			m_Renderer.Render();

			if (Keyboard::getKey(Keyboard::Key::F2)) {
				VulkanContext::GetDevice().WaitIdle();

				uint32_t width = m_Renderer.GetScreenSize().x;
				uint32_t height = m_Renderer.GetScreenSize().y;

				StagingBuffer stagingBuffer;
				stagingBuffer.Create(sizeof(float) * 4 * width * height, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
				UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
					m_Renderer.GetFinalImage().setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

					VkBufferImageCopy copyRegion = {};
					copyRegion.bufferOffset = 0;
					copyRegion.bufferRowLength = 0;
					copyRegion.bufferImageHeight = 0;

					copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copyRegion.imageSubresource.mipLevel = 0;
					copyRegion.imageSubresource.baseArrayLayer = 0;
					copyRegion.imageSubresource.layerCount = 1;
					copyRegion.imageExtent.width = width;
					copyRegion.imageExtent.height = height;
					copyRegion.imageExtent.depth = 1;

					//copy the buffer into the image
					vkCmdCopyImageToBuffer(cmd, m_Renderer.GetFinalImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &copyRegion);

					m_Renderer.GetFinalImage().setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
					});

				float* imagedata = (float*)stagingBuffer.Map();

				const auto now = std::chrono::system_clock::now();
				auto filename = std::format("screenshots/screenshot_{:%d-%m-%Y-%H-%M-%OS}.png", now);

				int bytes_per_pixel = 4;
				uint32_t bytes_per_scanline = bytes_per_pixel * width;
				uint8_t* imageOutput = new uint8_t[width * height * bytes_per_pixel];

				for (int i = 0; i < width * height * bytes_per_pixel; i++)				
					imageOutput[i] = (uint8_t)(imagedata[i] * 255.f);
				
				stbi_write_png(filename.c_str(), width, height, bytes_per_pixel, imageOutput, bytes_per_scanline);
				delete[] imageOutput;
				stagingBuffer.Unmap();
				stagingBuffer.Destroy();
			}
		}

		void RenderGUI(float deltaTime, const AssetManager& assetManager) {
			ImGui::Begin("Screen Render", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
			ImGui::GetBackgroundDrawList()->AddImage(m_Renderer.GetRenderTexture(), ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
			ImGui::End();

			if (debug_menu && renderGUI) {
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::Begin("Debug Menu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
				ImGui::Text(std::format("FPS: {0} FrameTime: {1}", (int)(1.f / deltaTime), deltaTime).c_str());
				ImGui::Text(std::format("Position: X:{0} Y:{1} Z:{2}", p.Position.x, p.Position.y, p.Position.z).c_str());
				ImGui::Text(std::format("Camera position: X:{0} Y:{1} Z:{2}", camera.Position.x, camera.Position.y, camera.Position.z).c_str());
				ImGui::Text(std::format("ChunkID: {}", getChunkID(getChunkPos(glm::ivec3(p.Position)))).c_str());
				ImGui::End();
			}


			if (console) {
				//input
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, 55));
				ImGui::Begin("Console Log", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
				if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
					ImGui::SetKeyboardFocusHere(0);
				ImGui::InputText("Log", consoleBuffer, std::size(consoleBuffer));
				ImGui::End();

				//history log
				ImGui::SetNextWindowPos(ImVec2(0, 55));
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y - 55));
				ImGui::Begin("History Log", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
				if (consoleHistory.size() > 0) {
					for (int i = consoleHistory.size() - 1; i >= 0; i--) {
						// fix memcpy
						if (ImGui::Button(consoleHistory[i].c_str())) memcpy(consoleBuffer, consoleHistory[i].c_str(), consoleHistory[i].size());
					}
				}
				ImGui::End();

			}

			if (renderGUI) {
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::Begin("crosshair", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				ImGui::SetCursorPos(ImVec2((window.GetSize().x - 20) / 2, (window.GetSize().y - 20) / 2));
				ImGui::Image(crosshair, ImVec2(20, 20));
				ImGui::End();

				//hotbar
				ImGui::SetNextWindowSize(ImVec2(1000, 100));
				ImGui::SetNextWindowPos(ImVec2(window.GetSize().x / 2 - 500, window.GetSize().y - 125));
				ImGui::Begin("HotBar", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
				
				for (int n = 0; n < 9; n++)
				{
					ImGui::PushID(n);
					//	if ((n % 10) != 0)
						ImGui::SameLine();
					if (p.inventory[n].itemID != 0)
						ImGui::Image(assetManager.m_Textures[itemData[p.inventory[n].itemID].textureID], ImVec2(90, 90));
					else
						ImGui::Button("", ImVec2(90, 90));
				
					// Our buttons are both drag sources and drag targets here!
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
					{
						// Set payload to carry the index of our item (could be anything)
						ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
						{
							IM_ASSERT(payload->DataSize == sizeof(int));
							int payload_n = *(const int*)payload->Data;
				
							auto tmp = p.inventory[n];
							p.inventory[n] = p.inventory[payload_n];
							p.inventory[payload_n] = tmp;
				
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::PopID();
				}
				ImGui::End();
			}

			if (inventory) {
				ImGui::SetNextWindowSize(ImVec2(1000, 500));
				ImGui::SetNextWindowPos(ImVec2((window.GetSize().x - 1000) / 2, (window.GetSize().y - 600) / 2));
				ImGui::Begin("Inventory", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
			
				for (int n = 0; n < p.inventory.size(); n++)
				{
					ImGui::PushID(n);
					if ((n % 10) != 0)
						ImGui::SameLine();
					//ImGui::Button(names[n], ImVec2(90, 90));
					if (p.inventory[n].itemID != 0)
						ImGui::ImageButton(assetManager.m_Textures[itemData[p.inventory[n].itemID].textureID], ImVec2(90, 90));
					else
						ImGui::Button("", ImVec2(90, 90));
					// Our buttons are both drag sources and drag targets here!
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
					{
						// Set payload to carry the index of our item (could be anything)
						ImGui::SetDragDropPayload("Item", &n, sizeof(int));
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Item");
						if (payload)
						{
							int payload_n = *(const int*)payload->Data;
			
							ItemSlot tmp = p.inventory[n];
							p.inventory[n] = p.inventory[payload_n];
							p.inventory[payload_n] = tmp;
			
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::PopID();
				}
			
			
				ImGui::End();
			}
		}

		void RenderImGuiEscapeMenu() {
			ImGui::SetNextWindowSize(ImVec2(400, 400));
			ImGui::SetNextWindowPos(ImVec2((window.GetSize().x - 400) / 2, (window.GetSize().y - 400) / 2));
			ImGui::Begin("Elemental World", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
			ImGui::Text("Escape Menu");
			if (ImGui::Button("Resume"))
				ChangeMenu(MenuMode::GAME);
			
			if (ImGui::Button("Save and exit to main menu")) { SaveWorld(); ChangeMenu(MenuMode::MAINMENU); }
			if (ImGui::Button("Open Settings")) ChangeMenu(MenuMode::SETTINGS);
			if (ImGui::Button("Save and exit to desktop")) { SaveWorld(); window.close(); }

			ImGui::End();
		}

		void ParseCommand(std::string& command) {
			consoleHistory.push_back(command);
			// Command parsing
			CommandParser cmdParser;
			CommandType commandType = cmdParser.getCommandType(command);
			
			switch (commandType) {
			case wc::CommandType::UNKNOWN:
				WC_ERROR("Unknow command!");
				break;
			case wc::CommandType::textMessage:
				WC_INFO(command);
				break;
			case wc::CommandType::fly:			
				p.flying = cmdParser.getArgument();
				p.wasOnGround = false;
				p.m_isOnGround = false;
				p.wasFalling = false;
			
				break;
			case wc::CommandType::collide:
				p.collision = cmdParser.getArgument();
				break;
			case wc::CommandType::set:
			{
				std::string type = cmdParser.getStringArgument();
				if (type == "speed") p.MovementSpeed = cmdParser.getArgument(1);
				else if (type == "time") angle = cmdParser.getArgument(1);
				else {
					WC_ERROR("Indentifier '{0}' is not recognized", type.c_str());
				}
			}

				break;
			case wc::CommandType::setBlock:
			{
				BlockID block = getBlockID(cmdParser.getStringArgument());
				setBlock({ cmdParser.getArgument(1) , cmdParser.getArgument(2) , cmdParser.getArgument(3) }, block, true, true);
			}
				break;
			case wc::CommandType::give:
			{
				std::string itemName = cmdParser.getStringArgument(0);
				p.inventory.AddItem({ getItemID(itemName) , 1 });
			}
				break;
			case wc::CommandType::getBlockID:
				WC_INFO(getBlock({ cmdParser.getArgument(0) , cmdParser.getArgument(1) , cmdParser.getArgument(2) }));
				break;
			}
			command.clear();
		}

		void OnInput(float deltaTime) {
			// MENU MANAGMENT

			if (!console && !inventory)
			{
				// GAMEPLAY
				float yaw = glm::radians(p.rotation.x);
				float yaw90 = glm::radians(p.rotation.x + 90.f);
				float addFOV = 0.f;
				if (window.getKey(Keyboard::Key::W)) { // Front
					float adder = 0.f;
					if (window.getKey(Keyboard::Key::LControl)) { adder = 40.f; /*addFOV = 10.f;*/ }
					else if (window.getKey(Keyboard::Key::LShift) && !p.flying) { adder = -2.f; }
					p.acceleration.x += glm::cos(yaw) * (p.MovementSpeed + adder);
					p.acceleration.z += glm::sin(yaw) * (p.MovementSpeed + adder);
				}

				else if (window.getKey(Keyboard::Key::S)) { // Back
					p.acceleration.x -= glm::cos(yaw) * p.MovementSpeed;
					p.acceleration.z -= glm::sin(yaw) * p.MovementSpeed;
				}
				if (window.getKey(Keyboard::Key::A)) { // Left
					p.acceleration.x -= glm::cos(yaw90) * p.MovementSpeed;
					p.acceleration.z -= glm::sin(yaw90) * p.MovementSpeed;
				}
				else if (window.getKey(Keyboard::Key::D)) { // Right
					p.acceleration.x += glm::cos(yaw90) * p.MovementSpeed;
					p.acceleration.z += glm::sin(yaw90) * p.MovementSpeed;
				}

				if (Keyboard::getKey(Keyboard::Key::F5) == GLFW_PRESS) {
					if (!thirdPerson) {
						camera.distanceFromCamera = 3.f;
						thirdPerson = true;
					}
					else if (thirdPerson) {
						camera.distanceFromCamera = 0.f;
						thirdPerson = false;
					}
				}

				if (window.getKey(Keyboard::Key::Space))
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
				else if (window.getKey(Keyboard::Key::LShift) && p.flying)
					p.acceleration.y -= p.MovementSpeed;

				if (window.getKey(Keyboard::Key::C)) { addFOV = -80.f; MouseSensitivity = Settings::ZoomMouseSensitivity; }
				else
					MouseSensitivity = Settings::MouseSensitivity;

				camera.FOV = glm::radians(addFOV + 90.f);

				if (scrollY != 0.f) {
					if (scrollY < 0) p.currentSlot++;
					else p.currentSlot--;
					if (p.currentSlot < 0) p.currentSlot = inventorySizeX - 1;
					else if (p.currentSlot > inventorySizeX - 1) p.currentSlot = 0;
				}

				glm::ivec2 t;

				glm::ivec2 pos = window.getCursorPos();

				t = window.GetSize() / 2;

				float ms = 1.f / MouseSensitivity;

				if (!Settings::InvertMouse) 
					p.rotation.x -= (t.x - pos.x) * ms;
				else
					p.rotation.x += (t.x - pos.x) * ms;

				p.rotation.y += (t.y - pos.y) * ms;

				// make sure that when pitch is out of bounds, screen doesn't get flipped
				if (p.rotation.y > 89.f) p.rotation.y = 89.f;
				else if (p.rotation.y < -89.f) p.rotation.y = -89.f;

				if (p.rotation.x > 360.f) p.rotation.x = 0.f;
				else if (p.rotation.x < 0.f) p.rotation.x = 360.f;

				window.setCursorPos(t);
			}

			if (Keyboard::getKey(Keyboard::Key::E) && !console) {
				inventory = !inventory;
				if (inventory)
					window.SetCursorMode(GLFW_CURSOR_NORMAL);
				else 
					window.SetCursorMode(GLFW_CURSOR_DISABLED);
			}

			if (Keyboard::getKey(Keyboard::Key::F1)) renderGUI = !renderGUI;
			
			if (Keyboard::getKey(Keyboard::Key::F8)) debug_menu = !debug_menu;
			if (Keyboard::getKey(Keyboard::Key::Enter) && console) {
				if (consoleBuffer[0] == '/') { 
					std::string command = consoleBuffer;
					ParseCommand(command);
				}
				memset(consoleBuffer, 0, sizeof(consoleBuffer));
			}
			if (Keyboard::getKey(Keyboard::Key::Enter)) { 
				console = !console; 
				if (console) window.SetCursorMode(GLFW_CURSOR_NORMAL);
				else window.SetCursorMode(GLFW_CURSOR_DISABLED);
			}
			

			// PLAYER RELATED
			p.velocity += p.acceleration;
			p.acceleration = { 0.f,0.f,0.f };

			if (!p.flying)
				p.velocity.y -= gravity * deltaTime;

			p.Position += p.velocity * deltaTime;

			if (!p.wasFalling && p.isFalling()) p.startOfFall = p.Position.y;
			if (!p.wasOnGround && p.m_isOnGround)
				if (p.startOfFall - p.Position.y > minFallDistance) p.health -= 1.5f;


			p.wasOnGround = p.m_isOnGround;
			p.wasFalling = p.isFalling();

			camera.Position = p.Position;
			camera.Position.y += p.Size.y - 0.1f;
			glm::vec3 vRayStart = camera.Position;
			camera.Yaw = p.rotation.x;
			camera.Pitch = p.rotation.y;

			p.velocity.x *= 0.009f;
			p.velocity.z *= 0.009f;
			if (p.flying)
				p.velocity.y *= 0.009f;
			//////////////

			camera.Update(window.getAspectRatio());

			bool bBreak = Mouse::getMouse(Mouse::Button::LEFT);
			bool bPlace = Mouse::getMouse(Mouse::Button::RIGHT);

			if (window.getKey(Keyboard::Key::Num1)) blockHolding = campfire;
			if (window.getKey(Keyboard::Key::Num2)) blockHolding = murshroom;

			float breakTime = 1.f / 12.f;
			if (bBreak && !startBreaking) {
				startBreaking = true;
				m_BlockBreakTimer.Start();
			}

			glm::vec4 color = glm::vec4(1.f);
			if (startBreaking) color = glm::mix(glm::vec4(1.f), glm::vec4(1.f, 0.f, 1.f, 1.f), m_BlockBreakTimer.getElapsedTime() / breakTime);

			float fMaxDistance = 6.f;

			glm::vec3 vRayLength1D = glm::vec3(0.f);
			glm::vec3& vRayDir = camera.Front;
			glm::ivec3 vMapCheck = glm::floor(vRayStart), vMapLastCheck = glm::floor(vRayStart), vStep = glm::ivec3(glm::sign(vRayDir));
			glm::vec3 vRayUnitStepSize = abs(1.f / vRayDir);

			// Establish Starting Conditions
			vRayLength1D[0] = (vRayDir[0] < 0.f ? (vRayStart[0] - float(vMapCheck[0])) : (float(vMapCheck[0]) + 1.f - vRayStart[0])) * vRayUnitStepSize[0];
			vRayLength1D[1] = (vRayDir[1] < 0.f ? (vRayStart[1] - float(vMapCheck[1])) : (float(vMapCheck[1]) + 1.f - vRayStart[1])) * vRayUnitStepSize[1];
			vRayLength1D[2] = (vRayDir[2] < 0.f ? (vRayStart[2] - float(vMapCheck[2])) : (float(vMapCheck[2]) + 1.f - vRayStart[2])) * vRayUnitStepSize[2];

			bool bTileFound = false;
			float fDistance = 0.f;
			while (!bTileFound && fDistance < fMaxDistance)
			{
				// Walk along shortest path
				int axis = 0;
				for (int i = 0; i < 3; i++) {
					int nextAxis = (i + 1) % 3;
					if (vRayLength1D[axis] > vRayLength1D[nextAxis]) axis = nextAxis;
				}

				vMapCheck[axis] += vStep[axis];
				fDistance = vRayLength1D[axis];
				vRayLength1D[axis] += vRayUnitStepSize[axis];

				// Test tile at new test point
				BlockID blockID = getBlock(vMapCheck);
				Block& block = m_BlockData[blockID];
				if (blockID > 0 && block.connectionType != ConnectionType::FLUID_CONNECT)
				{
					if (breakPos != vMapCheck) {
						startBreaking = false;
						breakPos = vMapCheck;
					}
					//DrawOutlineCube(vMapCheck, glm::vec3(1.f), color);
					if (m_BlockBreakTimer.getElapsedTime() >= breakTime && startBreaking) {
						p.inventory.PushItem({getBlock(vMapCheck), 1});
						setBlock(vMapCheck, 0, true, true);
						startBreaking = false;
					}
					else if (bPlace) 
						setBlock(vMapLastCheck, blockHolding, true, true);
					
					bTileFound = true;
				}
				vMapLastCheck = vMapCheck;
			}
		}

		std::string getChunkPath(const glm::ivec3& pos) const {
			return "worlds/" + worldName + "/Chunk data/Island 0/r." + std::to_string(pos.x) + "." + std::to_string(pos.y) + "." + std::to_string(pos.z) + ".ewr";
		}

		std::string GetEntityPath() const {
			return "worlds/" + worldName + "/Entity data/";
		}

		// SERIALIZATION/DESERIALIZATION
		void CreateNewWorld(const std::string& name) {
			worldName = name;
			std::filesystem::create_directories("worlds/" + worldName + "/Chunk data/Island 0");
			std::filesystem::create_directories("worlds/" + worldName + "/Entity data");
			SaveWorld();
			p.Serialize(GetEntityPath());
		}

		void LoadWorld() {
			p.Position = { 0, (RenderDistance * RenderDistance * 0.5f), 0 };
			p.Deserialize(GetEntityPath());

			glm::ivec3 offset = getChunkPos(p.Position) - glm::ivec3(chunkSize / 2);
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				chunks[chunkID].position = to3D(chunkID, RenderDistance) + offset;

				TryToLoadChunk(chunkID);
			}
			if (!multiPlayer) {
				YAML::Node config = YAML::LoadFile("worlds/" + worldName + "/world.properties");
				if (config["time"]) angle = config["time"].as<float>();
				if (config["seed"]) worldNoise.SetSeed(config["seed"].as<int>());
				m_WorldLoaded = true;
			}
		}

		void SaveWorld() {
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) SaveChunk(chunkID);

			YAML::Node config;
			config["seed"] = worldNoise.GetSeed();
			config["time"] = angle;
			YAMLUtils::saveFile("worlds/" + worldName + "/world.properties", config);

			//p.Serialize(GetEntityPath());
			m_WorldLoaded = false;
		}

	private:
		void TryToLoadChunk(const ChunkID& chunkID) {
			if (multiPlayer && m_ClientInstance.IsConnected()) {
				net::message<GameMsg> msg;
				msg.header.id = GameMsg::RequestChunk;
				glm::ivec3 position = chunks[chunkID].position;
				msg << position << chunkID;
				m_ClientInstance.Send(msg);
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
					generateTerrain = true;
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

		// WORLD GENERATION

		void GenerateChunkTerrain(const ChunkID& chunk) {
			memset(&chunks[chunk].data, 0, sizeof(chunks[chunk].data));
			auto setBlockTerrain = [&](const glm::ivec3& pos, BlockID blockID) {
				chunks[chunk].data[pos.x][pos.y][pos.z] = blockID;
			};

			for (uint32_t z = 0; z < chunkSize; z++) {
				for (uint32_t x = 0; x < chunkSize; x++) {
					glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].position.x * chunkSize, z + chunks[chunk].position.z * chunkSize);
					int heightMap = (int)worldNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
					float floraGen = treeNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y) * 0.5f + 0.5f;
					//float baseTemperature = temperatureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
					//float moisture = moistureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y) * 0.5f + 0.5f;
					int dirtDepth = (int)(floraGen * 3.f) + 2;

					for (uint8_t y = 0; y < chunkSize; y++) {
						glm::ivec3 pos = chunks[chunk].position * glm::ivec3(chunkSize) + glm::ivec3(x, y, z);
						//float temperature = baseTemperature * (1.f - pos.y / (worldNoise.GetMultiplier() - water_level));
						//uint32_t biome = getBiome(temperature, moisture);
						bool onSand = (pos.y <= water_level + (int)(floraGen * 3.f) && pos.y == heightMap);
						if (pos.y == heightMap) {
							if (onSand)
								setBlockTerrain(glm::ivec3(x, y, z), sand);
							else
								setBlockTerrain(glm::ivec3(x, y, z), grass);
						}

						if (pos.y < heightMap && pos.y >= heightMap - dirtDepth) setBlockTerrain(glm::ivec3(x, y, z), dirt);
						if (pos.y < heightMap - dirtDepth) setBlockTerrain(glm::ivec3(x, y, z), stone);
						if (pos.y > heightMap && pos.y < water_level) setBlockTerrain(glm::ivec3(x, y, z), water);
						//if (pos.y == 125) setBlockTerrain(glm::ivec3(x, y, z), grass, chunk, false, false);
					}
				}
			}

			chunks[chunk].canBeUpdated = true;
		}		

		// WORLD/CHUNK MANAGING

		void UpdateChunk(const ChunkID& chunkID) {
			Chunk& chunk = chunks[chunkID];
			BlockID data[chunkSize][chunkSize][chunkSize];
			memcpy(data, chunk.data, sizeof(data));
			glm::ivec3 chunkPos = chunk.position * glm::ivec3(chunkSize);

			glm::vec3 start = chunk.position * glm::ivec3(chunkSize);
			glm::vec3 end = start + glm::vec3(chunkSize);

			for (uint8_t y = 0; y < chunkSize; y++)
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) {
						BlockID block = data[x][y][z];
						if (block != 0) {
							chunk.empty = false;
							if (m_BlockData[block].connectionType == CUSTOM_MODEL) { // @TODO: add more shader-unsupported cases
								data[x][y][z] = 0; 
							}
							else {
								if (chunkPos.x + x < start.x) start.x = chunkPos.x + x;
								else if (chunkPos.x + x > end.x) end.x = chunkPos.x + x;
								
								if (chunkPos.y + y < start.y) start.y = chunkPos.y + y;
								else if (chunkPos.y + y > end.y) end.y = chunkPos.y + y;
								
								if (chunkPos.z + z < start.z) start.z = chunkPos.z + z;
								else if (chunkPos.z + z > end.z) end.z = chunkPos.z + z;
							}
						}
					}

			chunk.boxStart = start;
			chunk.boxEnd = end;

			m_Renderer.GetVoxelBuffer().SetData(data, sizeof(data), chunkID * sizeof(Chunk::data));
		}

		void setBlockLocal(const glm::uvec3& pos, BlockID blockID, const ChunkID& chunkID, bool playerEdited, bool replyToServer) {
			const uint32_t& x = pos.x;
			const uint32_t& y = pos.y;
			const uint32_t& z = pos.z;

			
			BlockID& chunkBlockID = chunks[chunkID].data[x][y][z];
			if (chunkBlockID == blockID) return;
			glm::vec3 globalPos = glm::vec3(chunks[chunkID].position) * glm::vec3(chunkSize) + glm::vec3(pos);
			glm::ivec3 iGlobalPos = globalPos;
			if (blockID == 0) {

				if (m_BlockData[chunkBlockID].connectionType == ConnectionType::CUSTOM_MODEL) {
					Mesh& mesh = blockMeshes[m_BlockData[chunkBlockID].meshID];
					m_Renderer.removeModel(mesh.cmd, globalPos);
				}
			}
			else {

				if (m_BlockData[blockID].connectionType == ConnectionType::CUSTOM_MODEL) {
					Mesh& mesh = blockMeshes[m_BlockData[blockID].meshID];

					m_Renderer.Draw(mesh.cmd, globalPos);
					//m_UpdateModels = true;
				}
			}			

			chunks[chunkID].data[x][y][z] = blockID;
			chunks[chunkID].canBeUpdated = true;

			if (replyToServer && multiPlayer) {
				net::message<GameMsg> msg;
				msg.header.id = GameMsg::BlockEdit;
				msg << blockID << iGlobalPos; // may be problems here
				m_ClientInstance.Send(msg);
			}
			else if (playerEdited) chunks[chunkID].used = true;
			updateChunks = true;
		}

		void setBlock(const glm::ivec3& pos, BlockID block, bool playerEdited, bool replyToServer) {
			int16_t chunk = getChunkID(getChunkPos(pos));
			if (chunk > -1) setBlockLocal(getBlockPos(pos), block, chunk, playerEdited, replyToServer);
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

		BlockID getBlockID(const std::string& name) {
			for (BlockID i = 0; i < m_BlockData.size(); i++)
				if (m_BlockData[i].name == name) return i;

			WC_WARN("Could not find {0}. Air block assumed.", name);
			return 0;
		}

		ItemID getItemID(const std::string& name) {
			for (BlockID i = 0; i < itemData.size(); i++)
				if (itemData[i].name == name) return i;

			WC_WARN("Could not find {0}.", name);
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
					glm::ivec3 pos = to3D(counter, chunkSize);
					uint8_t x = pos.x;
					uint8_t y = pos.z;
					uint8_t z = pos.y;
					setBlockLocal({ x,y,z }, block.first, chunk, false, false);
					counter++;
				}
			}
		}

		bool isInBox(const glm::vec3& point, const glm::vec3& start, const glm::vec3& end) {
			return (point.x >= start.x && point.x <= end.x) &&
				(point.y >= start.y && point.y <= end.y) &&
				(point.z >= start.z && point.z <= end.z);
		}

		void GenerateOctree(std::vector<ChunkNode>& nodes, uint32_t nodeID, uint32_t depth) {
			if (depth == 0) {
				// Base case: we have reached the maximum depth, so create a leaf node
				ChunkNode& node = nodes[nodeID];
				
				for (int i = 0; i < chunks.size(); i++) {
					Chunk& chunk = chunks[i];
				
					if (isInBox(chunk.boxStart, node.min, node.max) && isInBox(chunk.boxEnd, node.min, node.max)) {
						node.children[0] = i * chunkVolume;
						node.min = chunk.boxStart;
						node.max = chunk.boxEnd;
						node.parentID = nodeID;
						node.flags |= IS_LEAF_BIT;
						if (chunk.empty) node.flags |= IS_EMPTY_BIT;
					}					
				}
			}
			else {
				// Recursive case: create 8 child nodes and recursively generate their octrees
				glm::vec3 mid = (nodes[nodeID].min + nodes[nodeID].max) * 0.5f;
				bool empty = true;
				for (int i = 0; i < 8; i++) {
					glm::vec3 childStart = nodes[nodeID].min;
					glm::vec3 childEnd = nodes[nodeID].max;
					if (i & 1) { childStart.x = mid.x; }
					else { childEnd.x = mid.x; }
					if (i & 2) { childStart.y = mid.y; }
					else { childEnd.y = mid.y; }
					if (i & 4) { childStart.z = mid.z; }
					else { childEnd.z = mid.z; }

					ChunkNode childNode;

					childNode.min = childStart;
					childNode.max = childEnd;
					childNode.parentID = nodeID;

					uint32_t childNodeID = nodes.size();
					nodes.push_back(childNode);

					nodes[nodeID].children[i] = childNodeID;

					GenerateOctree(nodes, childNodeID, depth - 1);

					if (!bool(nodes[childNodeID].flags & IS_EMPTY_BIT)) empty = false;
				}

				if (empty) nodes[nodeID].flags |= IS_EMPTY_BIT;
			}
		}
	};
}