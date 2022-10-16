#pragma once

#include <pch.h>
#include "Chunk.h"
#include "Biome.h"
#include <Maths/Frustum.h>
#include <FastNoise/FastNoiseLite.h>
#include "../entities/Player.h"
#include "../Game Mechanics/Model/Animation.h"
#include "../Game Mechanics/CommandParser.h"
<<<<<<< Updated upstream
#include <Utils/Memory.h>
=======

#include <wc/Utils/Memory.h>
#include <wc/Framebuffer.h>
#include <wc/Utils/DeletionQueue.h>
#include <wc/Maths/Frustum.h>

#include "../Rendering/AssetManager.h"
#include "../Rendering/LineBatcher.h"
#include "../Rendering/Renderer2D.h"
#include "../Rendering/Renderer3D.h"
>>>>>>> Stashed changes

namespace wc {
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

		Timer blockBreakTimer;
		bool startBreaking = false;
		glm::ivec3 breakPos = glm::ivec3(0);
		bool thirdPerson = false;

		AABB box;
		// Graphics
		LineBatcher lineBatcher;

		gl::Shader skyShader;
		float rotateSpeed = 1.f * 0.6f; // one cycle is one unit (in minutes)
		float angle = 0.f;

		Frustum viewFrustum;
		gl::Shader chunkShader;

		gl::UniformBuffer seneDataBuffer;

		gl::UniformBuffer lightBuffer;
		gl::ShaderStorageBuffer materialsBuffer;
		gl::ShaderStorageBuffer blockTransformBuffer;
		glm::vec4* blockTransforms = nullptr;
		uint32_t currentLightID = 0;
		uint32_t transformOffset = 0;

		struct SceneData {
			glm::mat4 ViewProj = glm::mat4(1.f);
			glm::vec3 cameraPos = glm::vec3(0.f);
			alignas(16) glm::vec3 lower_left_corner = glm::vec3(0.f);
			alignas(16) glm::vec3 horizontal = glm::vec3(0.f);
			alignas(16) glm::vec3 vertical = glm::vec3(0.f);
			alignas(16) glm::vec2 windowSize = glm::vec2(0.f);
			uint32_t numLights = 0;
			uint32_t transformOffset = 0;
		};

		gl::Buffer globalVertexBuffer;
		gl::Buffer globalIndexBuffer;
		gl::DrawIndirectBuffer indirectBuffer;
		gl::DrawElementsIndirectCommand* cmds = nullptr;
		std::array<Chunk, RenderDistance* RenderDistance* RenderDistance> chunks;

		FastNoiseLite worldNoise;
		FastNoiseLite treeNoise;
		FastNoiseLite caveNoise;

		bool generateTerrain : 1;
		int8_t water_level = 0;

		uint32_t localPlayerID = 0;
		std::unordered_map<uint32_t, PlayerDescription> players;

		// Data managing
		std::string worldName = "New world";
		//Animation animation;
		Model model;
		gl::UniformBuffer animationBuffer;

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
						if (blockID) file << (int)blockID << " " << x - start.x << " " << y - start.y << " " << z - start.z << "\n";
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
		};
		Light* lights = nullptr;
		uint32_t maxLights = chunkVolume;
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
		bool renderGUI = true;
		Player p;

		void Create() {
			chunkShader.Create("resourcepacks/" + resourceName + "/shaders/chunkShader.vert", "resourcepacks/" + resourceName + "/shaders/chunkShader.frag");
			skyShader.Create("resourcepacks/" + resourceName + "/shaders/skybox.vert", "resourcepacks/" + resourceName + "/shaders/skybox.frag");
			bloomShader.Create("resourcepacks/" + resourceName + "/shaders/bloomShader.comp");
			compositeShader.Create("resourcepacks/" + resourceName + "/shaders/composite.comp");

			chunkShader.depthTest = true;

			uint32_t bits = GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT;
			seneDataBuffer.Create(sizeof(SceneData), GL_DYNAMIC_STORAGE_BIT);
			seneDataBuffer.BufferBase(0);

			lightBuffer.Create(maxLights * sizeof(Light), bits);
			lightBuffer.BufferBase(1);

			materialsBuffer.Create(materialData.byte_size(), bits);
			materialsBuffer.BufferBase(3);

			bloomUBO.Create(sizeof(BloomUBOSettings), GL_DYNAMIC_STORAGE_BIT);
			bloomUBO.BufferBase(4);

			animationBuffer.Create(sizeof(glm::mat4) * (MAX_BONE_WEIGHTS + 1), GL_DYNAMIC_STORAGE_BIT);
			animationBuffer.BufferBase(5);

			blockTransformBuffer.Create(sizeof(glm::vec4) * chunks.size() * chunkVolume, bits);
			blockTransformBuffer.BufferBase(6);

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
			//worldGenState.new_enum("FractalType", {{ "None", FastNoiseLite::FractalType::FractalType_None },{ "None", FastNoiseLite::FractalType::FractalType_None } });
			worldGenState.script_file("scripts/worldGen.lua");
			if (worldGenState["noise"].valid()) worldNoise = worldGenState["noise"];
			if (worldGenState["TreeNoise"].valid()) treeNoise = worldGenState["TreeNoise"];
			if (worldGenState["CaveNoise"].valid()) caveNoise = worldGenState["CaveNoise"];

			if (worldGenState["water_level"].valid()) water_level = worldGenState["water_level"];

			assets.Create(30, 32, 32);
			blockData.counter = 1;
			materialData.Data = (Material*)materialsBuffer.Map(bits, materialData.allocated_size());
			materialData.counter = 1;
			itemData.counter = 1;

			//Loading blocks
			for (auto& p : std::filesystem::directory_iterator("scripts/blockScripts")) {
				std::string filename = p.path().stem().string();
				if (p.is_regular_file()) { //AddBlockScript
					std::string script = "scripts/blockScripts/" + filename + ".yaml";
					std::string conType;
					YAML::Node blockState = YAML::LoadFile(script);

					Block block;
					Material blockMaterial;

					if (blockState["name"]) block.name = blockState["name"].as<std::string>();
					else WC_WARN("No block name is specified in '{0}'. Block name 'air' assumed.", script);

					if (blockState["isCollidable"]) block.isCollidable = blockState["isCollidable"].as<bool>();
					if (blockState["ConnectionType"]) conType = blockState["ConnectionType"].as<std::string>();
					if (blockState["color"]) blockMaterial.color = blockState["color"].as<uint32_t>();
					if (blockState["cull"]) if (blockState["cull"].as<bool>()) blockMaterial.flags |= WC_CULL_BIT;

					for (uint8_t i = 0; i < ConnectionType::NON_EXISTENT; i++)
						if (conType == magic_enum::enum_name((ConnectionType)i)) block.connectionType = (ConnectionType)i;

					std::string diffusePath = "resourcepacks/" + resourceName + "/textures/block/diffuse/";
					std::string materialPath = "resourcepacks/" + resourceName + "/textures/block/materials/";

					if (blockState["allTextures"]) {
						std::string filename = blockState["allTextures"].as<std::string>();
						blockMaterial.albedo[0] = assets.LoadTexture(diffusePath + filename);
						for (int i = 1; i < 6; i++) blockMaterial.albedo[i] = blockMaterial.albedo[0];

						if (blockState["materialData"]) {
							blockMaterial.materialData[0] = assets.LoadTextureMaterial(materialPath + filename);
							for (int i = 1; i < 6; i++) blockMaterial.materialData[i] = blockMaterial.materialData[0];
						}
					}
					// @TODO: Remove
					else if (blockState["modelTexture"]) {
						std::string filename = blockState["modelTexture"].as<std::string>();
						blockMaterial.albedo[0] = assets.LoadModelTexture(diffusePath + filename);
						for (int i = 1; i < 6; i++) blockMaterial.albedo[i] = blockMaterial.albedo[0];
						
						if (blockState["materialData"]) {
							blockMaterial.materialData[0] = assets.LoadModelTextureMaterial(materialPath + filename);
							for (int i = 1; i < 6; i++) blockMaterial.materialData[i] = blockMaterial.materialData[0];
						}
					}
					else {
						for (uint32_t i = 0; i < (uint32_t)BlockTexture::LENGTH; i++) {
							auto name = std::string(magic_enum::enum_name((BlockTexture)i));
							if (blockState[name]) 
								blockMaterial.albedo[i] = assets.LoadTexture(diffusePath + blockState[name].as<std::string>());
						}

					}
					if (blockState["emitLight"]) block.emitLight = blockState["emitLight"].as<bool>();

					if (block.connectionType == ConnectionType::CUSTOM_MODEL) blockMaterial.flags |= WC_MODEL_BIT;
					block.material = materialData.push_back(blockMaterial);
					if (blockState["modelPath"]) {
						std::string path = blockState["modelPath"].as<std::string>();
						block.meshID = blockMeshes.size();
						blockMeshes[block.meshID].Load("resourcepacks/" + resourceName + "/models/" + path, block.material);
						blockMeshes.counter++;
					}


					blockData.push_back(block);
				}
			}
			assets.Free();

			lineBatcher.Create();
			chunkShader.VertexAttribPointer(0, 3, offsetof(Vertex, Position));  // position attribute
			chunkShader.VertexAttribPointer(1, 3, offsetof(Vertex, TexCoords)); // texture coord attribute
			chunkShader.VertexAttribPointer(2, 3, offsetof(Vertex, Normal)); // type attribute
			chunkShader.VertexAttribPointer(3, 1, offsetof(Vertex, materialID)); // color attribute

			indirectBuffer.Create(chunks.size() * sizeof(gl::DrawElementsIndirectCommand), bits);
			globalVertexBuffer.Create(MaxVertexCount * sizeof(Vertex) * chunks.size(), bits);
			globalIndexBuffer.Create(MaxIndexCount * sizeof(uint32_t) * chunks.size(), bits);

			cmds = (gl::DrawElementsIndirectCommand*)indirectBuffer.Map(bits, chunks.size() * sizeof(gl::DrawElementsIndirectCommand));
			globalVertices = (Vertex*)globalVertexBuffer.Map(bits, MaxVertexCount * sizeof(Vertex) * chunks.size());

			lights = (Light*)lightBuffer.Map(bits, maxLights * sizeof(Light));
			blockTransforms = (glm::vec4*)blockTransformBuffer.Map(bits, sizeof(glm::vec4) * chunks.size() * chunkVolume);

			for (ChunkID i = 0; i < chunks.size(); i++) {
				//Configuring the vertex buffer
				cmds[i].baseVertex = i * MaxVertexCount;
			}

			uint32_t iOffset = 0;
			uint32_t* indices = (uint32_t*)globalIndexBuffer.Map(bits, MaxIndexCount * sizeof(uint32_t));
			for (int i = 0; i < MaxIndexCount; i += 6) {
				indices[i + 0] = 0 + iOffset;
				indices[i + 1] = 1 + iOffset;
				indices[i + 2] = 2 + iOffset;

				indices[i + 3] = 2 + iOffset;
				indices[i + 4] = 3 + iOffset;
				indices[i + 5] = 0 + iOffset;

				iOffset += 4;
			}
			globalIndexBuffer.UnMap();

			chunkShader.SetVertexBuffer(globalVertexBuffer, sizeof(Vertex));
			chunkShader.SetIndexBuffer(globalIndexBuffer);

			model.Create("resourcepacks/default/models/player_model.obj");
			//animation.Create("resourcepacks/default/models/dancing_vampire.dae", model);

			addLight(glm::vec3(0.f), convertColor(glm::vec4(1.f, 1.f, 1.f, 0.f)));

			grass = getBlockID("grass_block");
			stone = getBlockID("stone_block");
			water = getBlockID("water");
			sand = getBlockID("sand");
			oak = getBlockID("wood");
			leaves = getBlockID("leaves");
			coal = getBlockID("campfire");
			dirt = getBlockID("dirt");

			box.position = glm::vec3(0.f, 128.f, 0.f);
			box.size = glm::vec3(1.f);
		}

		// Common blocks
		BlockID grass = 0;
		BlockID stone = 0;
		BlockID water = 0;
		BlockID sand = 0;
		BlockID oak = 0;
		BlockID leaves = 0;
		BlockID coal = 0;
		BlockID dirt = 0;

		void Join(const std::string& ip, const std::string& playerName) {
			if (multiPlayer) {
				clientInstance.Connect(ip, 60000);
				p.name = playerName;
			}
			LoadWorld();
		}

		void CreateScreen() {
			// Creating the screen framebuffer
<<<<<<< Updated upstream
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
=======
			wc::AttachmentCreateInfo attachmentInfo = {};
			attachmentInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // GL_RGBA32F
			attachmentInfo.width = window.GetSize().x;
			attachmentInfo.height = window.GetSize().y;
			attachmentInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			uint32_t attachment = framebuffer.addAttachment(attachmentInfo);
			
			wc::AttachmentCreateInfo depthAttachmentInfo = {};
			depthAttachmentInfo.format = RendererContext::GetDepthBuffer().GetFormat();
			depthAttachmentInfo.width = window.GetSize().x;
			depthAttachmentInfo.height = window.GetSize().y;
			depthAttachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			framebuffer.addAttachment(depthAttachmentInfo);
			
			framebuffer.Create(window.GetSize());
			
			scrTexture.Create(framebuffer.attachments[attachment].image, framebuffer.attachments[attachment].view);
			finalImage.Create(window.GetSize(), VK_FORMAT_R32G32B32A32_SFLOAT, 4, false, VK_IMAGE_USAGE_STORAGE_BIT);

			finalImage.GetImage().layout = VK_IMAGE_LAYOUT_GENERAL;
			UploadContext::immediate_submit([&](VkCommandBuffer cmd) {finalImage.GetImage().setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL); });
			VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			
			sampler.magFilter = VK_FILTER_LINEAR;
			sampler.minFilter = VK_FILTER_LINEAR;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler.minLod = 0.f;
			sampler.maxLod = 0.25f;
			sampler.compareEnable = false;
			sampler.anisotropyEnable = false;
			sampler.mipLodBias = 0.f;
			
			scrTexture.SetSamplerInfo(sampler);
			finalImage.SetSamplerInfo(sampler);
			render_interface.AddTextureFramebuffer(finalImage);




			bloomTexSize = glm::ivec2(window.GetSize().x, window.GetSize().y) / 2;
>>>>>>> Stashed changes
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
				//bloomBuffers[i].GenerateMipMap();
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

		bool RectVsRect(const AABB& r1, const AABB& r2)
		{
			return (r1.position.x < r2.position.x + r2.size.x && r1.position.x + r1.size.x > r2.position.x &&
				r1.position.y < r2.position.y + r2.size.y && r1.position.y + r1.size.y > r2.position.y &&
				r1.position.z < r2.position.z + r2.size.z && r1.position.z + r1.size.z > r2.position.z
				);
		}

		void Update(const float& deltaTime) {
			screen.Bind();
			glClear(GL_DEPTH_BUFFER_BIT);
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
							generateTerrain = true;
							break;
						}
						}
					}
					for (auto& player : players) {
						if (player.second.nUniqueID != localPlayerID) {
							lineBatcher.DrawOutlineCube(player.second.Position - p.Size, p.Size * 2.f, glm::vec4(1.f));
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
			SceneData sceneData;
			sceneData.windowSize = window.GetSize();
			sceneData.ViewProj = glm::perspective(camera.FOV, sceneData.windowSize.x / sceneData.windowSize.y, 0.1f, 1100.f) * camera.GetViewMatrix();
			sceneData.cameraPos = camera.Position;
			sceneData.lower_left_corner = camera.lower_left_corner;
			sceneData.vertical = camera.vertical;
			sceneData.horizontal = camera.horizontal;
			sceneData.numLights = currentLightID;
			seneDataBuffer.SetData(sizeof(SceneData), &sceneData);

			lights[0].vector = -glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)));

			// Draw sky
			skyShader.use();
			glDrawArrays(GL_TRIANGLES, 0, 3);

			angle += deltaTime * rotateSpeed;
			angle = glm::mod(angle, 360.f);

			viewFrustum.update(sceneData.ViewProj);
			uint32_t chunkHalf = chunkSize / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position); // @TODO: hmmm? why doesnt it work with glm::ivec3?
			for (ChunkID i = 0; i < chunks.size(); i++) {
				glm::ivec3& currChunkPos = chunks[i].position;
				glm::ivec3 newPosition = currChunkPos;
				if (currChunkPos.x < currentPlayerPos.x - chunkHalf) newPosition.x = currentPlayerPos.x + chunkHalf - 1;
				else if (currChunkPos.x > currentPlayerPos.x + chunkHalf) newPosition.x = currentPlayerPos.x - chunkHalf + 1;

				else if (currChunkPos.y < currentPlayerPos.y - chunkHalf) newPosition.y = currentPlayerPos.y + chunkHalf - 1;
				else if (currChunkPos.y > currentPlayerPos.y + chunkHalf) newPosition.y = currentPlayerPos.y - chunkHalf + 1;

				else if (currChunkPos.z < currentPlayerPos.z - chunkHalf) newPosition.z = currentPlayerPos.z + chunkHalf - 1;
				else if (currChunkPos.z > currentPlayerPos.z + chunkHalf) newPosition.z = currentPlayerPos.z - chunkHalf + 1;

				if (newPosition != currChunkPos) // Reset Chunk
				{
					SaveChunk(i);
					chunks[i].position = newPosition;
					UpdateNeighbours(i);
					TryToLoadChunk(i);

					chunks[i].canBeUpdated = true; // @TODO: try to remove this
				}
			}

			if (generateTerrain) {
				for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
					if (!chunks[chunk].generated) { GenerateChunkTerrain(chunk); chunks[chunk].generated = true; }

				for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
					if (!chunks[chunk].generatedStructures) { GenerateChunkStructures(chunk); chunks[chunk].generatedStructures = true; }

				generateTerrain = false;
			}

			for (ChunkID i = 0; i < chunks.size(); i++) {

				if (chunks[i].canBeUpdated) { UpdateMesh(i); chunks[i].canBeUpdated = false; }

				bool show = false;

				if (viewFrustum.isBoxInFrustum(AABB(chunks[i].position * glm::ivec3(chunkSize), glm::vec3(chunkSize))) && cmds[i].count > 0) show = true;

				cmds[i].instanceCount = show;
			}

			assets.Bind();
			chunkShader.use();
			chunkShader.SetVertexBuffer(globalVertexBuffer, sizeof(Vertex));
			chunkShader.SetIndexBuffer(globalIndexBuffer);
			indirectBuffer.Bind();
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, chunks.size(), sizeof(gl::DrawElementsIndirectCommand));
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

			assets.BindModelData();
			for (uint32_t i = 0; i < blockMeshes.size(); i++) {
				BlockMesh& mesh = blockMeshes[i];
				if (mesh.cmd.instanceCount > 0) {
					chunkShader.SetIndexBuffer(mesh.indexBuffer);
					chunkShader.SetVertexBuffer(mesh.vertexBuffer, sizeof(Vertex));
					glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, &mesh.cmd);
					sceneData.transformOffset += mesh.cmd.instanceCount;
					seneDataBuffer.SetData(sizeof(SceneData), &sceneData);
				}
			}


			if (RectVsRect(box, AABB(p.Position - p.Size, p.Size * 2.f)))
				lineBatcher.DrawOutlineCube(box, convertColor(0xFF33FF00) * glm::vec4(3.f));
			else
				lineBatcher.DrawOutlineCube(box, convertColor(0xFF33FF00));

			lineBatcher.Flush(renderGUI);

			//animationBuffer.SetData(sizeof(glm::mat4) * MAX_BONE_WEIGHTS, animation.GetPoseTransforms());

			// render the loaded model
			//animation.Update(deltaTime);
			glm::mat4 Model = glm::mat4(1.f);
			Model = glm::translate(Model, p.Position - glm::vec3(0.f, p.Size.y, 0.f));    // translate it down so it's at the center of the scene
			//Model = glm::scale(Model, glm::vec3(0.8f));
			Model = glm::rotate(Model, glm::radians(camera.Yaw + 90.f), glm::vec3(0.f, -1.f, 0.f));

			animationBuffer.SetData(sizeof(glm::mat4), glm::value_ptr(Model), sizeof(glm::mat4) * MAX_BONE_WEIGHTS);

			if (thirdPerson)
				model.Draw();

			screen.unbind();
			// GUI		

			RenderBloom();

			finalImage.BindTextureImage(0, GL_WRITE_ONLY);
			scrTexture.Bind(1); // use the color attachment texture as the texture of the quad plane	
			bloomBuffers[2].Bind(2);
			compositeShader.use();
			compositeShader.Dispatch(glm::ceil((glm::vec2)sceneData.windowSize / glm::vec2(m_BloomComputeWorkGroupSize)));
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			render_interface.DrawQuad({ 0,0 }, sceneData.windowSize, finalImage);
			if (renderGUI) render_interface.DrawQuad((sceneData.windowSize - 15.f) / 2.f, { 15, 15 }, render_interface.whiteTexture);
		}


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
			else if (commandType == CommandType::UNKNOWN) WC_ERROR("Unknow command!");
			command = "";
		}

		void OnInput(const float& deltaTime) {
			// MENU MANAGMENT

			//if (/*!textbox.isSelected*/true)
			{
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

				camera.FOV = glm::radians(addFOV + 90.f);

				if (scrollY != 0.f) {
					if (scrollY < 0) p.currentSlot++;
					else p.currentSlot--;
					if (p.currentSlot < 0) p.currentSlot = inventorySizeX - 1;
					else if (p.currentSlot > inventorySizeX - 1) p.currentSlot = 0;
				}

				glm::ivec2 t;

				glm::ivec2 pos = Mouse::GetMousePosToWindow();

				t = window.GetSize() / 2;

				float ms = 1.f / MouseSensitivity;

				p.rotation.x -= (t.x - pos.x) * ms;
				p.rotation.y += (t.y - pos.y) * ms;

				// make sure that when pitch is out of bounds, screen doesn't get flipped
				if (p.rotation.y > 89.f) p.rotation.y = 89.f;
				else if (p.rotation.y < -89.f) p.rotation.y = -89.f;

				if (p.rotation.x > 360.f) p.rotation.x = 0.f;
				else if (p.rotation.x < 0.f) p.rotation.x = 360.f;

				Mouse::SetMousePosition(t);
			}

			if (wc::Keyboard::getKey(wc::Keyboard::Key::F2)) {
				glm::ivec2 size = finalImage.GetSize();
				uint32_t byteSize = size.x * size.y * 4;
				uint8_t* data = new uint8_t[byteSize];
				glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
				stbi_flip_vertically_on_write(true);
				stbi_write_png("screenshots/screenshot.png", size.x, size.y, 4, data, size.x * 4);
				delete[] data;
			}

			if (wc::Keyboard::getKey(wc::Keyboard::Key::F1)) renderGUI = !renderGUI;

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

			camera.Update();

			bool bBreak = Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT);
			bool bPlace = Mouse::getMouse(GLFW_MOUSE_BUTTON_RIGHT);

			float breakTime = 1.f / 12.f;
			if (bBreak && !startBreaking) {
				startBreaking = true;
				blockBreakTimer.Start();
			}

			glm::vec4 color = glm::vec4(1.f);
			if (startBreaking) color = glm::mix(glm::vec4(1.f), glm::vec4(1.f, 0.f, 1.f, 1.f), blockBreakTimer.getElapsedTime() / breakTime);

			float fMaxDistance = 6.f;

			glm::ivec3 vMapCheck = glm::floor(vRayStart), vMapLastCheck = glm::floor(vRayStart), vStep = glm::ivec3(0);
			glm::vec3 vRayLength1D = glm::vec3(0.f);
			glm::vec3& vRayDir = camera.Front;
			glm::vec3 vRayUnitStepSize = abs(1.f / vRayDir);

			// Establish Starting Conditions
			for (int i = 0; i < 3; i++) {
				if (vRayDir[i] < 0)
				{
					vStep[i] = -1;
					vRayLength1D[i] = (vRayStart[i] - float(vMapCheck[i])) * vRayUnitStepSize[i];
				}
				else
				{
					vStep[i] = 1;
					vRayLength1D[i] = (float(vMapCheck[i] + 1) - vRayStart[i]) * vRayUnitStepSize[i];
				}
			}

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
				Block& block = blockData[blockID];
				if (blockID > 0 && block.connectionType != ConnectionType::FLUID_CONNECT)
				{
					if (breakPos != vMapCheck) {
						startBreaking = false;
						breakPos = vMapCheck;
					}
					lineBatcher.DrawOutlineCube(vMapCheck, glm::vec3(1.f), color);
					//DrawOutlineCube(vRayStart + vRayDir * fDistance - glm::vec3(0.05f), glm::vec3(0.1f), glm::vec4(1.f));
					if (blockBreakTimer.getElapsedTime() >= breakTime && startBreaking) {
						//p.inventory.AddItem(block - 1, p.currentSlot);
						setBlock(vMapCheck, 0, true, true);
						startBreaking = false;
					}
					else if (bPlace) {
						//ItemID itemID = p.inventory.data[p.currentSlot].itemID;
						//if (p.inventory.RemoveItem(p.currentSlot))
						setBlock(vMapLastCheck, /*items[itemID].block*/coal, true, true);
					}
					bTileFound = true;
				}
				vMapLastCheck = vMapCheck;
			}
		}

		void Destroy() {
			if (multiPlayer) clientInstance.Disconnect();
			else SaveWorld();
		}

	private:

		std::string getChunkPath(const glm::ivec3& pos) {
			return "worlds/" + worldName + "/Chunk data/Island 0/r." + std::to_string(pos.x) + "." + std::to_string(pos.y) + "." + std::to_string(pos.z) + ".ewr";
		}

		// SERIALIZATION/DESERIALIZATION
		void CreateNewWorld(const std::string& name) {
			worldName = name;
			std::filesystem::create_directories("worlds/" + worldName + "/Chunk data/Island 0");
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

			for (int x2 = x - 2; x2 < x + 3; x2++)
				for (int y2 = y - 3 + trunkHeight - 1; y2 < y + 2 + trunkHeight - 1; y2++)
					for (int z2 = z - 2; z2 < z + 3; z2++)
						setBlock(glm::ivec3(x2, y2, z2) + (int)chunkSize * chunks[chunk].position, leaves, false, false);

			for (int i = 0; i < trunkHeight; i++)
				setBlock(glm::ivec3(x, y + i, z) + (int)chunkSize * chunks[chunk].position, oak, false, false);
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {
			memset(&chunks[chunk].data, 0, sizeof(chunks[chunk].data));
			auto setBlockTerrain = [&](const glm::ivec3& pos, const BlockID& blockID) {
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


			{ int16_t neg = chunks[chunk].neighborNeg[0]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			{ int16_t neg = chunks[chunk].neighborNeg[1]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			{ int16_t neg = chunks[chunk].neighborNeg[2]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }

			{ int16_t Pos = chunks[chunk].neighborPos[0]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			{ int16_t Pos = chunks[chunk].neighborPos[1]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			{ int16_t Pos = chunks[chunk].neighborPos[2]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
		}

		void GenerateChunkStructures(const ChunkID& chunk) {
			for (uint32_t z = 0; z < chunkSize; z++) {
				for (uint8_t x = 0; x < chunkSize; x++) {
					glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].position.x * chunkSize, z + chunks[chunk].position.z * chunkSize);
					int heightMap = (int)worldNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
					float treeGen = treeNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);

					for (uint8_t y = 0; y < chunkSize; y++) {
						glm::ivec3 pos = chunks[chunk].position * glm::ivec3(chunkSize) + glm::ivec3(x, y, z);
						uint32_t type = getBiome(0.75f, 0.f);
						float CaveNoise = caveNoise.GetNoise((float)pos.x, (float)pos.y, (float)pos.z);
						bool onGrass = !(pos.y <= water_level + (int)(treeGen * 2.f) && pos.y == heightMap);

						if (pos.y == heightMap + 1 && heightMap + 1 > water_level && onGrass && !(CaveNoise >= 0.25f && CaveNoise <= 0.99f))
							if (treeGen <= 0.49f && treeGen > 0.48f && x % 5 == 0) GenerateTree(x, y, z, treeGen, chunk);
					}
				}
				return;
			}
		}

		uint32_t getBiome(const float& temperature, const float& moisture = 0.f) {
			for (uint32_t i = 1; i < biomeMap.size(); i++) {
				if (temperature >= biomeMap[i].minTemp && temperature <= biomeMap[i].maxTemp
					//&& moisture >= biomeMap[i].minMois && moisture <= biomeMap[i].maxMois
					) return i;
			}
			return 0;
		}

		// LIGHT MANAGING (deprecated)
		uint32_t addLight(const glm::vec3& position, const uint32_t& color) {
			uint32_t light = currentLightID;
			if (currentLightID <= maxLights) {
				lights[currentLightID].vector = position;
				lights[currentLightID].color = color;
				currentLightID++;
			}
			return light;
		}

		void removeLight(const glm::vec3& position) {
			for (uint32_t i = 0; i < maxLights; i++)
				if (lights[i].vector == position) {
					currentLightID--;
					lights[i] = lights[currentLightID];
					break;
				}
		}

		// WORLD/CHUNK MANAGING
		void setBlockLocal(const glm::ivec3& pos, const BlockID& blockID, const ChunkID& chunkID, const bool& playerEdited, const bool& replyToServer) {
			uint16_t x = pos.x;
			uint16_t y = pos.y;
			uint16_t z = pos.z;

			BlockID& chunkBlockID = chunks[chunkID].data[x][y][z];
			if (chunkBlockID == blockID) return;
			glm::vec3 globalPos = glm::vec3(chunks[chunkID].position) * glm::vec3(chunkSize) + glm::vec3(pos);
			glm::ivec3 iGlobalPos = globalPos;
			bool sortModels = false;
			if (blockID == 0) {
				if (blockData[chunkBlockID].emitLight)
					removeLight(globalPos + glm::vec3(0.5f));

				if (blockData[chunkBlockID].connectionType == ConnectionType::CUSTOM_MODEL) {
					BlockMesh& mesh = blockMeshes[blockData[chunkBlockID].meshID];

					for (uint32_t i = 0; i < chunks.size(); i++)
						if (glm::vec3(blockTransforms[i]) == globalPos) {
							transformOffset--;
							blockTransforms[i] = blockTransforms[transformOffset];
							mesh.cmd.instanceCount--;
							break;
						}
					sortModels = true;
				}
			}
			else {
				if (blockData[blockID].emitLight)
					addLight(globalPos + glm::vec3(0.5f), convertColor(glm::vec4(1.f)));

				if (blockData[blockID].connectionType == ConnectionType::CUSTOM_MODEL) {
					BlockMesh& mesh = blockMeshes[blockData[blockID].meshID];

					blockTransforms[transformOffset] = glm::vec4(globalPos, blockData[blockID].meshID);
					transformOffset++;
					mesh.cmd.instanceCount++;
					sortModels = true;
				}
			}

			if (sortModels) {
				// @TODO: maybe improve at some point in the future + transfer this to a compute shader
				for (uint32_t i = 0; i < transformOffset; i++)
					for (uint32_t j = 0; j < transformOffset - i - 1; j++) {
						if (blockTransforms[j].w > blockTransforms[j + 1].w)
							std::swap(blockTransforms[j], blockTransforms[j + 1]);
					}
			}

			chunks[chunkID].data[x][y][z] = blockID;
			chunks[chunkID].canBeUpdated = true;

			if (replyToServer && multiPlayer) {
				net::message<GameMsg> msg;
				msg.header.id = GameMsg::BlockEdit;
				msg << blockID << iGlobalPos; // may be problems here
				clientInstance.Send(msg);
			}
			else if (playerEdited) chunks[chunkID].used = true;


			if (x == 0) { int16_t neg = chunks[chunkID].neighborNeg[0]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (y == 0) { int16_t neg = chunks[chunkID].neighborNeg[1]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (z == 0) { int16_t neg = chunks[chunkID].neighborNeg[2]; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }

			if (x == chunkSize - 1) { int16_t Pos = chunks[chunkID].neighborPos[0]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (y == chunkSize - 1) { int16_t Pos = chunks[chunkID].neighborPos[1]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (z == chunkSize - 1) { int16_t Pos = chunks[chunkID].neighborPos[2]; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block, const bool& playerEdited, const bool& replyToServer) {
			int16_t chunk = getChunkID(getChunkPos(pos));
			if (chunk > -1) setBlockLocal(getBlockPos(pos), block, chunk, playerEdited, replyToServer);
		}

		// should be replaced with UpdateChunk
		void UpdateMesh(const ChunkID& chunkID) {
			uint32_t offset = 0;
			Chunk& chunk = chunks[chunkID];
			cmds[chunkID].count = 0;

			glm::ivec3 chunkPos = chunk.position * glm::ivec3(chunkSize);

			Vertex* vertices = (Vertex*)(globalVertices + chunkID * MaxVertexCount);

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
							else if (chunk.neighborPos[d] > -1) {
								xq[d] = 0;
								checkBlock = chunks[chunk.neighborPos[d]].data[xq.x][xq.y][xq.z];
								checkType = blockData[checkBlock].connectionType;
							}
							// The mask is set to true if there is a visible face between two blocks, i.e. both aren't empty and both aren't blocks							
							Block& checkBlockData = blockData[checkBlock];
							Block& BlockData = blockData[blockID];
							if (type != checkType && type != ConnectionType::NON_EXISTENT && checkType != ConnectionType::NON_EXISTENT) {
								if (blockID != checkBlock && type == ConnectionType::CONNECT_DEFAULT) { mask[n] = blockID + 1; textureMask[n] = materialData[BlockData.material].albedo[d]; }
								else if ((blockID == 0 || type != ConnectionType::CONNECT_DEFAULT) && checkType == ConnectionType::CONNECT_DEFAULT) { mask[n] = checkBlock + 1; textureMask[n] = materialData[checkBlockData.material].albedo[d + 3]; }

								else if (blockID != checkBlock && type == ConnectionType::NO_CONNECT) { mask[n] = blockID + 1; textureMask[n] = materialData[BlockData.material].albedo[d]; }
								else if (blockID == 0 && checkType == ConnectionType::NO_CONNECT) { mask[n] = checkBlock + 1; textureMask[n] = materialData[checkBlockData.material].albedo[d + 3]; }

								else if (checkBlock == 0 && type == ConnectionType::FLUID_CONNECT) { mask[n] = blockID + 1; textureMask[n] = materialData[BlockData.material].albedo[d]; }
								else if (blockID == 0 && checkType == ConnectionType::FLUID_CONNECT) { mask[n] = checkBlock + 1; textureMask[n] = materialData[checkBlockData.material].albedo[d + 3]; }
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
								glm::vec3 corner[4];

								if (d == 0) { // @TODO: Wrong!
									corner[2] = x;           // Top-left vertice position
									corner[3] = x + du;      // Top right vertice position
									corner[1] = x + dv;      // Bottom left vertice position
									corner[0] = x + du + dv; // Bottom right vertice position
								}
								else if (d == 1) {
									corner[0] = x;                 // Top-left vertice position
									corner[1] = x + du;         // Top right vertice position
									corner[3] = x + dv;         // Bottom left vertice position
									corner[2] = x + du + dv; // Bottom right vertice position
								}
								else if (d == 2) {
									corner[2] = x;                 // Top-left vertice position
									corner[1] = x + du;         // Top right vertice position
									corner[3] = x + dv;         // Bottom left vertice position
									corner[0] = x + du + dv; // Bottom right vertice position
									std::swap(h1, w1);
								}
								glm::vec3 normal = glm::normalize(glm::cross(corner[2] - corner[0], corner[1] - corner[0]));
								if (cmds[chunkID].count < MaxIndexCount) {
									BlockID blockID = mask[n] - 1;
									glm::vec2 TexCoords[] = {
										glm::vec2(0.f, 0.f),
										glm::vec2(0.f, w1),
										glm::vec2(h1,  w1),
										glm::vec2(h1,  0.f),
									};

									uint32_t texID = 0;
									if (normal == glm::vec3(0.f, -1.f, 0.f)) texID = (uint32_t)BlockTexture::TOP;
									if (normal == glm::vec3(0.f,  1.f, 0.f)) texID = (uint32_t)BlockTexture::BOTTOM;
									if (normal == glm::vec3( 1.f, 0.f, 0.f)) texID = (uint32_t)BlockTexture::RIGHT;
									if (normal == glm::vec3(-1.f, 0.f, 0.f)) texID = (uint32_t)BlockTexture::LEFT;
									if (normal == glm::vec3(0.f, 0.f,  1.f)) texID = (uint32_t)BlockTexture::FRONT;
									if (normal == glm::vec3(0.f, 0.f, -1.f)) texID = (uint32_t)BlockTexture::BACK;

									for (uint32_t i = 0; i < ARRAYSIZE(corner); i++)
										vertices[i + offset] = Vertex(corner[i] + (glm::vec3)chunkPos, { TexCoords[i], texID }, normal, blockData[blockID].material);

									cmds[chunkID].count += 6;
									offset += ARRAYSIZE(corner);
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
		}

		// Deprecated
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

			for (ChunkID i = 0; i < chunks.size(); i++) {
				if (chunks[i].position == neighborXpos) { chunks[chunk].neighborPos[0] = i; chunks[chunks[chunk].neighborPos[0]].neighborNeg[0] = chunk; }
				if (chunks[i].position == neighborYpos) { chunks[chunk].neighborPos[1] = i; chunks[chunks[chunk].neighborPos[1]].neighborNeg[1] = chunk; }
				if (chunks[i].position == neighborZpos) { chunks[chunk].neighborPos[2] = i; chunks[chunks[chunk].neighborPos[2]].neighborNeg[2] = chunk; }

				if (chunks[i].position == neighborXneg) { chunks[chunk].neighborNeg[0] = i; chunks[chunks[chunk].neighborNeg[0]].neighborPos[0] = chunk; }
				if (chunks[i].position == neighborYneg) { chunks[chunk].neighborNeg[1] = i; chunks[chunks[chunk].neighborNeg[1]].neighborPos[1] = chunk; }
				if (chunks[i].position == neighborZneg) { chunks[chunk].neighborNeg[2] = i; chunks[chunks[chunk].neighborNeg[2]].neighborPos[2] = chunk; }

				if (chunks[chunk].neighborPos[0] != -1 && chunks[chunk].neighborPos[1] != -1 && chunks[chunk].neighborPos[2] != -1 &&
					chunks[chunk].neighborNeg[0] != -1 && chunks[chunk].neighborNeg[1] != -1 && chunks[chunk].neighborNeg[2] != -1) return;
			}
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

		// BLOOM
		void RenderBloom()
		{
			bloomShader.use();
			BloomUBOSettings settings;
			settings.Params = glm::vec4(bloomSettings.Threshold, bloomSettings.Threshold - bloomSettings.Knee, bloomSettings.Knee * 2.f, 0.25f / bloomSettings.Knee);
			bloomUBO.SetData(sizeof(BloomUBOSettings), &settings);
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
				bloomUBO.SetData(sizeof(BloomUBOSettings), &settings);

				bloomBuffers[1].BindTextureImage(0, GL_WRITE_ONLY, currentMip);
				bloomShader.Dispatch(mipSize);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				// Pong 
				settings.LOD = currentMip;
				bloomUBO.SetData(sizeof(BloomUBOSettings), &settings);

				bloomBuffers[0].BindTextureImage(0, GL_WRITE_ONLY, currentMip);
				bloomBuffers[1].Bind(1);
				bloomShader.Dispatch(mipSize);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}

			// First Upsample		
			settings.LOD = mips - 2;
			settings.Mode = (int)BloomMode::UpsampleFirst;
			bloomUBO.SetData(sizeof(BloomUBOSettings), &settings);

			bloomBuffers[2].BindTextureImage(0, GL_WRITE_ONLY, mips - 1);
			bloomBuffers[0].Bind(1);

			bloomShader.Dispatch(glm::ceil((glm::vec2)bloomBuffers[2].GetMipSize(mips - 1) / glm::vec2(m_BloomComputeWorkGroupSize)));
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			bloomBuffers[2].Bind(2);
			settings.Mode = (int)BloomMode::Upsample;
			for (int currentMip = mips - 2; currentMip >= 0; currentMip--)
			{
				settings.LOD = currentMip;
				bloomUBO.SetData(sizeof(BloomUBOSettings), &settings);

				bloomBuffers[2].BindTextureImage(0, GL_WRITE_ONLY, currentMip);

				bloomShader.Dispatch(glm::ceil((glm::vec2)bloomBuffers[2].GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize)));
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}
		}
	};
}