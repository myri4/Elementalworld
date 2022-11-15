#pragma once

#include <pch.h>
#include "../Rendering/AssetManager.h"
#include "Chunk.h"
#include "Biome.h"
#include <FastNoise/FastNoiseLite.h>
#include "../entities/Player.h"
#include "../Rendering/Model/Animation.h"
#include "../Game Mechanics/CommandParser.h"

#include <wc/Utils/Memory.h>
#include <wc/Shader.h>
#include <wc/Framebuffer.h>
#include <wc/Utils/DeletionQueue.h>

#include "../Rendering/Renderer3D.h"
#include <stb_image/stb_write.h>
#include "../Settings.h"

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
		bool debug_menu = true;
		Texture m_RenderTexture;
		DeletionQueue delQueue;
		// Player related
		Camera camera;
		float MouseSensitivity = 5.f;
		float gravity = 20.f;

		Timer m_BlockBreakTimer;
		bool startBreaking = false;
		glm::ivec3 breakPos = glm::ivec3(0);
		bool thirdPerson = false;

		// Graphics
		Texture crosshair;

		float rotateSpeed = 1.f * 0.6f; // one cycle is one unit (in minutes)
		float angle = 0.f;
		wc::Shader skyShader;

		Frustum viewFrustum;
		bool m_UpdateModels = false;
		bool m_SortModels = false;

		wc::Buffer m_SceneDataBuffer;

		wc::Buffer m_LightBuffer;
		wc::Buffer m_MaterialsBuffer;
		wc::Buffer m_BlockTransformBuffer;

		wc::CPUBufferManager<glm::vec4> m_BlockTransforms;

		struct SceneData {
			glm::mat4 ViewProj = glm::mat4(1.f);
			glm::vec3 cameraPos = glm::vec3(0.f);
			alignas(16) glm::vec3 lower_left_corner = glm::vec3(0.f);
			alignas(16) glm::vec3 horizontal = glm::vec3(0.f);
			alignas(16) glm::vec3 vertical = glm::vec3(0.f);
			alignas(16) glm::vec2 windowSize = glm::vec2(0.f);
			uint32_t numLights = 0;
			uint32_t bvhCounter = 0;
		};

		wc::Buffer m_IndirectBuffer; // indirect buffer
		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> chunks;

		FastNoiseLite worldNoise;
		FastNoiseLite treeNoise;

		bool generateTerrain : 1;
		int8_t water_level = 0;

		uint32_t localPlayerID = 0;
		std::unordered_map<uint32_t, PlayerDescription> players;

		// Data managing
		bool m_WorldLoaded = false;
		// Multiplayer
		bool m_WaitingForConnection = true;
		net::client_interface<GameMsg> m_ClientInstance;

		struct Light {
			glm::vec3 vector;
			uint32_t color;
		};
		wc::CPUBufferManager<Light> lights;
		uint32_t maxLights = chunkVolume;
		// Composite stuff
		wc::FramebufferWC m_Framebuffer;
		wc::Image scrTexture;
		wc::ImageView screenImageView;
		wc::Sampler screenSampler;


		wc::Image finalImage;
		wc::ComputeShader compositeShader;

		wc::ComputeShader rayTracingShader;
		wc::Buffer chunkBVHBuffer;
		uint32_t m_BvhCounter = 0;
	public:
		std::string worldName = "New world";
		bool multiPlayer = false;
		bool renderGUI = true;
		Player p;

		void Create(const glm::vec2& windowSize) {

			if (!std::filesystem::exists("worlds")) std::filesystem::create_directory("worlds");
			if (!std::filesystem::exists("cache")) std::filesystem::create_directory("cache");
			if (!std::filesystem::exists("settings.yaml")) Settings::Save();

			Settings::Load();

			CreateScreen();

			m_SceneDataBuffer.Create(sizeof(SceneData), wc::UNIFORM_BUFFER);
			m_LightBuffer.Create(maxLights * sizeof(Light), wc::UNIFORM_BUFFER);
			m_MaterialsBuffer.Create(materialData.byte_size(), wc::STORAGE_BUFFER);
			m_BlockTransformBuffer.Create(sizeof(glm::vec4) * chunks.size() * chunkVolume, wc::STORAGE_BUFFER);
			chunkBVHBuffer.Create(sizeof(ChunkAABB) * chunks.size(), wc::STORAGE_BUFFER);		
			

			delQueue.push_function([=] { m_SceneDataBuffer.Destroy(); });
			delQueue.push_function([=] { m_LightBuffer.Destroy(); });
			delQueue.push_function([=] { m_MaterialsBuffer.Destroy(); });
			delQueue.push_function([=] { m_BlockTransformBuffer.Destroy(); });
			delQueue.push_function([=] { chunkBVHBuffer.Destroy(); });
						
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
			worldGenState.script_file("scripts/worldGen.lua");
			if (worldGenState["noise"].valid()) worldNoise = worldGenState["noise"];
			if (worldGenState["TreeNoise"].valid()) treeNoise = worldGenState["TreeNoise"];
			
			if (worldGenState["water_level"].valid()) water_level = worldGenState["water_level"];
			
			wc::StagingBuffer matBuffer;
			matBuffer.Create(materialData.byte_size());
			
			blockData.counter = 1;
			materialData.Data = (Material*)matBuffer.Map();
			materialData.counter = 1;
			itemData.counter = 1;
			
			std::string diffusePath = GetAssetPath() + "/textures/block/diffuse/";
			std::string materialPath = GetAssetPath() + "/textures/block/materials/";
			
			Renderer3D::CreateMesh(MaxVertexCount * (uint32_t)chunks.size(), MaxIndexCount * (uint32_t)chunks.size());
			//Loading blocks
			std::vector<Vertex> modelVertices;
			std::vector<uint32_t> modelIndices;
			for (auto& p : std::filesystem::directory_iterator("scripts/blockScripts")) {
				std::string filename = p.path().stem().string();
				if (p.is_regular_file()) { //AddBlockScript
					std::string script = "scripts/blockScripts/" + filename + ".yaml";
					YAML::Node blockState = YAML::LoadFile(script);
			
					Block block;
					Material material;
			
					if (blockState["name"]) block.name = blockState["name"].as<std::string>();
					else WC_WARN("No block name is specified in '{0}'. Block name 'air' assumed.", script);
			
					if (blockState["isCollidable"]) block.isCollidable = blockState["isCollidable"].as<bool>();
					if (blockState["ConnectionType"]) block.connectionType = magic_enum::enum_cast<ConnectionType>(blockState["ConnectionType"].as<std::string>()).value();
					if (blockState["color"]) material.color = blockState["color"].as<uint32_t>();
					if (blockState["cull"]) if (blockState["cull"].as<bool>()) material.flags |= WC_CULL_BIT;
			
					if (blockState["allTextures"]) {
						material.albedo[0] = assets.LoadTexture(diffusePath + blockState["allTextures"].as<std::string>());
						for (int i = 1; i < 6; i++) material.albedo[i] = material.albedo[0];
					}
					else {
						for (uint32_t i = 0; i < magic_enum::enum_count<BlockTexture>(); i++) {
							auto name = std::string(magic_enum::enum_name((BlockTexture)i));
							if (blockState[name]) 
								material.albedo[i] = assets.LoadTexture(diffusePath + blockState[name].as<std::string>());
						}
					}
					if (blockState["emitLight"]) block.emitLight = blockState["emitLight"].as<bool>();
			
					if (blockState["modelPath"]) { 
						material.flags |= WC_MODEL_BIT;
						block.connectionType = ConnectionType::CUSTOM_MODEL;
					}
			
					if (blockState["materialData"]) {
						material.materialData[0] = assets.LoadTextureMaterial(materialPath + blockState["materialData"].as<std::string>());
						for (int i = 1; i < 6; i++) material.materialData[i] = material.materialData[0];
					}
			
					block.materialID = materialData.push_back(material);
					if (blockState["modelPath"]) {
						std::string path = blockState["modelPath"].as<std::string>();
						block.meshID = blockMeshes.size();
						uint32_t id = block.meshID;
						blockMeshes[id].Load(GetAssetPath() + "/models/" + path, block.materialID, modelVertices, modelIndices);
						blockMeshes.counter++;
					}
			
					blockData.push_back(block);
				}
			}
			assets.LoadAll();
			matBuffer.Unmap();
			m_MaterialsBuffer.SetData(matBuffer, materialData.byte_size());
			matBuffer.Destroy();
			

			m_IndirectBuffer.Create(chunks.size() * sizeof(VkDrawIndexedIndirectCommand), wc::INDIRECT_BUFFER);
			delQueue.push_function([=] { m_IndirectBuffer.Destroy(); });

			Renderer3D::BuildBuffers(window.GetExtent());
			Renderer3D::vertexBuffer.SetData(modelVertices.data(), sizeof(Vertex) * modelVertices.size(), sizeof(Vertex) * MaxVertexCount * chunks.size());
			Renderer3D::indexBuffer.SetData(modelIndices.data(), sizeof(uint32_t) * modelIndices.size(), sizeof(uint32_t) * MaxIndexCount * chunks.size());

						
			lights.Create(maxLights * sizeof(Light));
			delQueue.push_function([=] { lights.Unmap(); lights.Destroy(); });
			lights.Map();
			
			m_BlockTransforms.Create(sizeof(glm::vec4) * chunks.size() * chunkVolume);
			delQueue.push_function([=] { m_BlockTransforms.Unmap(); m_BlockTransforms.Destroy(); });
			m_BlockTransforms.Map();

			uint32_t iOffset = 0;
			wc::CPUBuffer <uint32_t> indices;
			indices.Create(MaxIndexCount * sizeof(uint32_t));
			indices.Map();
			for (uint32_t i = 0; i < MaxIndexCount; i += 6) {
				indices[i + 0] = 0 + iOffset;
				indices[i + 1] = 1 + iOffset;
				indices[i + 2] = 2 + iOffset;
			
				indices[i + 3] = 2 + iOffset;
				indices[i + 4] = 3 + iOffset;
				indices[i + 5] = 0 + iOffset;
			
				iOffset += 4;
			}
			indices.Unmap();
			
			Renderer3D::indexBuffer.SetData(indices.GetBuffer(), MaxIndexCount * sizeof(uint32_t));
			indices.Destroy();

			crosshair.Load(GetAssetPath() + "/textures/misc/cursor.png");
			//model.Create("resourcepacks/default/models/player_model.obj", screen.renderPass, windowSize);
			//animation.Create("resourcepacks/default/models/dancing_vampire.dae", model);
			
			addLight(glm::vec3(0.f), convertColor(glm::vec4(1.f, 0.891f, 0.796f, 0.f)));
			
			grass = getBlockID("grass_block");
			stone = getBlockID("stone_block");
			water = getBlockID("water");
			sand = getBlockID("sand");
			oak = getBlockID("wood");
			leaves = getBlockID("leaves");
			coal = getBlockID("coal_ore");
			dirt = getBlockID("dirt");
			campfire = getBlockID("campfire");
			murshroom = getBlockID("murshroom");
			blockHolding = campfire;

			CreateDynamicPipelines();

			ChunkAABB aabb;
			aabb.start = glm::vec4(128.f);
			aabb.end = glm::vec4(129.f);
			chunkBVHBuffer.SetData(&aabb, sizeof(aabb), sizeof(ChunkAABB) * m_BvhCounter);
			m_BvhCounter++;
		}

		void CreateDynamicPipelines() {
			auto windowSize = window.GetSize();
			Renderer3D::CreateLinePipeline(m_Framebuffer.renderPass, m_SceneDataBuffer.GetDescriptorInfo());
			{
				wc::ShaderCreateInfo createInfo;
				createInfo.vertexShader = GetAssetPath() + "/shaders/chunkShader.vert";
				createInfo.fragmentShader = GetAssetPath() + "/shaders/chunkShader.frag";
				createInfo.windowSize = windowSize;
				createInfo.renderPass = m_Framebuffer.renderPass;
				createInfo.vertexDescription = Vertex::get_vertex_description();
				createInfo.blending = true;
				createInfo.depthTest = true;
				createInfo.cachePath = wc::GetCachedAssetPath() + "/shaders/Renderer3D.bin";
				Renderer3D::shader.Create(createInfo);

				wc::DescriptorWriter writer;
				writer.dstSet = Renderer3D::shader.descriptorSet;
				writer.write_buffer(0, m_SceneDataBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.write_buffer(1, m_LightBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.write_buffer(2, m_MaterialsBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
					.write_buffer(3, m_BlockTransformBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
					.write_image(4, assets.texArr.GetDescriptorData(assets.sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.write_image(5, assets.textureMaterialArr.GetDescriptorData(assets.sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

				writer.Update();
			}
			{
				wc::ShaderCreateInfo createInfo;
				createInfo.vertexShader = GetAssetPath() + "/shaders/skybox.vert";
				createInfo.fragmentShader = GetAssetPath() + "/shaders/skybox.frag";
				createInfo.windowSize = windowSize;
				createInfo.renderPass = m_Framebuffer.renderPass;
				createInfo.blending = false;
				createInfo.depthTest = false;
				createInfo.cachePath = GetCachedAssetPath() + "/shaders/skyShader.bin";
				skyShader.Create(createInfo);

				wc::DescriptorWriter writer;
				writer.dstSet = skyShader.descriptorSet;
				writer.write_buffer(0, m_SceneDataBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.write_buffer(1, m_LightBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

				writer.Update();
			}

			// Compute pipelines
			{

				bloomShader.Create(GetAssetPath() + "/shaders/bloomShader.comp");


				GenerateBloomDescriptor(m_BloomBuffers[0].imageViews[0], screenImageView);

				for (uint32_t currentMip = 1; currentMip < m_BloomMipLevels; currentMip++)
				{

					// Ping 
					GenerateBloomDescriptor(m_BloomBuffers[1].imageViews[currentMip], m_BloomBuffers[0].imageViews[0]);

					// Pong 
					GenerateBloomDescriptor(m_BloomBuffers[0].imageViews[currentMip], m_BloomBuffers[1].imageViews[0]);
				}

				// First Upsample
				GenerateBloomDescriptor(m_BloomBuffers[2].imageViews[m_BloomMipLevels - 1], m_BloomBuffers[0].imageViews[0]);

				for (int currentMip = m_BloomMipLevels - 2; currentMip >= 0; currentMip--)
					GenerateBloomDescriptor(m_BloomBuffers[2].imageViews[currentMip], m_BloomBuffers[0].imageViews[0]);
			}
			{
				compositeShader.Create(GetAssetPath() + "/shaders/composite.comp");

				wc::DescriptorWriter writer;
				writer.dstSet = compositeShader.descriptorSet;
				writer
					.write_image(0, GetDescriptorData(screenSampler, screenImageView, finalImage), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
					.write_image(1, GetDescriptorData(screenSampler, screenImageView, scrTexture), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.write_image(2, GetDescriptorData(bloomImageSampler, m_BloomBuffers[2].imageViews[0], VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

				writer.Update();
			}

			{
				rayTracingShader.Create(GetAssetPath() + "/shaders/rayTracingShader.comp");

				wc::DescriptorWriter writer;
				writer.dstSet = rayTracingShader.descriptorSet;
				writer.write_buffer(0, m_SceneDataBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.write_image(1, GetDescriptorData(screenSampler, screenImageView, scrTexture), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
					.write_buffer(2, chunkBVHBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

				writer.Update();
			}
		}

		void DestroyDynamicPipelines() {
			Renderer3D::shader.Destroy();
			Renderer3D::DestroyLinePipeline();
			skyShader.Destroy();
			rayTracingShader.Destroy();
			compositeShader.Destroy();
			bloomShader.Destroy();
		}

		void Destroy() {
			if (multiPlayer) m_ClientInstance.Disconnect();
			else if (m_WorldLoaded) SaveWorld();
			delQueue.flush();
			Renderer3D::Destroy();
			skyShader.SaveCache();
			skyShader.Destroy();
			rayTracingShader.Destroy();
			compositeShader.Destroy();
			bloomShader.Destroy();
			assets.Destroy();

			DestroyScreen();

			Settings::Save();
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
		BlockID campfire = 0;
		BlockID murshroom = 0;

		BlockID blockHolding = 0;

		void Join(const std::string& ip, const std::string& playerName) {
			if (multiPlayer) {
				m_ClientInstance.Connect(ip, 60000);
				p.name = playerName;
			}
			LoadWorld();
		}

		void CreateScreen() {
			// Creating the screen framebuffer
			wc::AttachmentCreateInfo attachmentInfo = {};
			attachmentInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // GL_RGBA32F
			attachmentInfo.width = window.GetSize().x;
			attachmentInfo.height = window.GetSize().y;
			attachmentInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			uint32_t attachment = m_Framebuffer.addAttachment(attachmentInfo);
			
			wc::AttachmentCreateInfo depthAttachmentInfo = {};
			depthAttachmentInfo.format = Renderer3D::depthBuffer.GetFormat();
			depthAttachmentInfo.width = window.GetSize().x;
			depthAttachmentInfo.height = window.GetSize().y;
			depthAttachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			m_Framebuffer.addAttachment(depthAttachmentInfo);
			
			m_Framebuffer.Create(window.GetSize());
			
			scrTexture = m_Framebuffer.attachments[attachment].image;
			screenImageView = m_Framebuffer.attachments[attachment].view;
			{
				//allocate and create the image
				VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

				info.imageType = VK_IMAGE_TYPE_2D;

				info.format = VK_FORMAT_R32G32B32A32_SFLOAT;

				info.extent.width = static_cast<uint32_t>(window.GetSize().x);
				info.extent.height = static_cast<uint32_t>(window.GetSize().y);
				info.extent.depth = 1;

				info.mipLevels = 1;
				info.arrayLayers = 1;
				info.samples = VK_SAMPLE_COUNT_1_BIT;
				info.tiling = VK_IMAGE_TILING_OPTIMAL;
				info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

				finalImage.Create(info);

				finalImage.layout = VK_IMAGE_LAYOUT_GENERAL;
				UploadContext::immediate_submit([&](VkCommandBuffer cmd) {finalImage.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL); });
			}
			scrTexture.layout = VK_IMAGE_LAYOUT_GENERAL;
			
			glm::ivec2 bloomTexSize = glm::ivec2(window.GetSize().x, window.GetSize().y) / 2;
			bloomTexSize += glm::ivec2(m_BloomComputeWorkGroupSize - bloomTexSize.x % m_BloomComputeWorkGroupSize, m_BloomComputeWorkGroupSize - bloomTexSize.y % m_BloomComputeWorkGroupSize);
			m_BloomMipLevels = scrTexture.GetMipLevelCount() - 4;
			
			SamplerCreateInfo sampler;
			
			sampler.magFilter = Filter::LINEAR;
			sampler.minFilter = Filter::LINEAR;
			sampler.mipmapMode = SamplerMipmapMode::LINEAR;
			sampler.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
			sampler.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
			sampler.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;
			sampler.minLod = 0.f;
			sampler.maxLod = float(m_BloomMipLevels);

			screenSampler.Create(sampler);
			bloomImageSampler.Create(sampler);


			for (int i = 0; i < 3; i++) {
				m_BloomBuffers[i].Create(bloomTexSize.x, bloomTexSize.y, m_BloomMipLevels);
				m_BloomBuffers[i].image.SetName("m_BloomBuffers[" + std::to_string(i) + "]");
			}			

			m_RenderTexture.Create(finalImage, screenSampler, screenImageView);
		}

		void DestroyScreen() {
			bloomImageSampler.Destroy();
			for (int i = 0; i < 3; i++) m_BloomBuffers[i].Destroy();			
			m_Framebuffer.Destroy();
			m_Framebuffer.DestroyAttachments();
			screenSampler.Destroy();

			finalImage.Destroy();
		}

		void Update(const float& deltaTime) {
			{
			wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			VkRenderPassBeginInfo fRPInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };

			fRPInfo.renderPass = m_Framebuffer.renderPass;
			fRPInfo.renderArea.offset.x = 0;
			fRPInfo.renderArea.offset.y = 0;
			fRPInfo.renderArea.extent = window.GetExtent();
			fRPInfo.framebuffer = m_Framebuffer.framebuffer;

			VkClearValue clearValuesFB[2];

			VkClearValue& clearValueFB = clearValuesFB[0];
			clearValueFB.color = { { 0.0f, 0.0f, 0.f, 0.0f } };

			VkClearValue& depthValue = clearValuesFB[1];
			depthValue.depthStencil.depth = 1.f;

			fRPInfo.clearValueCount = std::size(clearValuesFB);
			fRPInfo.pClearValues = clearValuesFB;
			//once we start adding rendering commands, they will go here
			m_Framebuffer.renderPass.Begin(cmd, fRPInfo);

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
							net::message<GameMsg> msg;
							msg.header.id = GameMsg::Client_RegisterWithServer;
							PlayerDescription descPlayer;
							msg << descPlayer;
							m_ClientInstance.Send(msg);
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
							chunks[chunkID].canBeUpdated = true;
							generateTerrain = true;
							break;
						}
						}
					}
					for (auto& player : players) {
						if (player.second.nUniqueID != localPlayerID) {
							Renderer3D::DrawOutlineCube(player.second.Position - p.Size, p.Size * 2.f, glm::vec4(1.f));
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

			// camera/view transformation
			SceneData sceneData;
			sceneData.windowSize = window.GetSize();
			sceneData.ViewProj = glm::perspective(camera.FOV, sceneData.windowSize.x / sceneData.windowSize.y, 0.1f, 1100.f) * camera.GetViewMatrix();
			sceneData.cameraPos = camera.Position;
			sceneData.lower_left_corner = camera.lower_left_corner;
			sceneData.vertical = camera.vertical;
			sceneData.horizontal = camera.horizontal;
			sceneData.numLights = lights.GetCounter();
			sceneData.bvhCounter = m_BvhCounter;
			m_SceneDataBuffer.SetData(&sceneData, sizeof(SceneData));

			lights[0].vector = -glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)));

			viewFrustum.update(sceneData.ViewProj);

			angle += deltaTime * rotateSpeed;
			angle = glm::mod(angle, 360.f);

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
			
				//for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
				//	if (!chunks[chunk].generatedStructures) { GenerateChunkStructures(chunk); chunks[chunk].generatedStructures = true; }
			
				generateTerrain = false;
			}

			uint32_t size = lights.GetCounter() ? lights.GetCounter() : 1;
			lights.Unmap();
			m_LightBuffer.SetData(lights.GetBuffer(), size * sizeof(Light));
			lights.Map();

			for (ChunkID i = 0; i < chunks.size(); i++) if (chunks[i].canBeUpdated) { UpdateMesh(i); chunks[i].canBeUpdated = false; }

			if (m_UpdateModels) {
				m_UpdateModels = false;

				if (m_SortModels) {
					for (uint32_t i = 0; i < m_BlockTransforms.GetCounter(); i++)
						for (uint32_t j = 0; j < m_BlockTransforms.GetCounter() - i - 1; j++) {
							if (m_BlockTransforms[j].w > m_BlockTransforms[j + 1].w)
								std::swap(m_BlockTransforms[j], m_BlockTransforms[j + 1]);
						}
					m_SortModels = false;
				}

				size = m_BlockTransforms.GetCounter() ? m_BlockTransforms.GetCounter() : 1;
				m_BlockTransforms.Unmap();
				m_BlockTransformBuffer.SetData(m_BlockTransforms.GetBuffer(), sizeof(glm::vec4) * size * chunkVolume);
				m_BlockTransforms.Map();
			}

			// Draw sky
			if (Settings::sky) {
				skyShader.Bind(cmd);
				cmd.Draw(3);
			}
			
			Renderer3D::Bind(cmd);

			uint32_t defaultVal = 0;
			cmd.PushConstants(Renderer3D::shader.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t), &defaultVal);

			cmd.DrawIndexedIndirect(m_IndirectBuffer, (uint32_t)chunks.size());
			
			uint32_t shaderTransformOffset = 0;
			for (uint32_t i = 0; i < blockMeshes.size(); i++) {
				BlockMesh& mesh = blockMeshes[i];
				if (mesh.cmd.instanceCount > 0) {
			
					cmd.DrawIndexedIndirect(mesh.cmd);
					shaderTransformOffset += mesh.cmd.instanceCount;
					cmd.PushConstants(Renderer3D::shader.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t), &shaderTransformOffset);
				}
			}
			
			Renderer3D::Flush(renderGUI);

			m_Framebuffer.renderPass.End(cmd);
			cmd.End();


			VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

			submit.commandBufferCount = 1;
			submit.pCommandBuffers = cmd.GetPointer();

			submit.pSignalSemaphores = RendererContext::computeSemaphore.GetPointer();
			submit.signalSemaphoreCount = 1;

			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			submit.pWaitDstStageMask = &waitStage;

			//@TODO: prerecord command buffers and remove fences everywhere possible
			VulkanContext::graphicsQueue.Submit(submit, VK_NULL_HANDLE);

			//RendererContext::renderFence.Wait();
			//RendererContext::renderFence.Reset();
			
			//cmd.Reset();
			}

			// compute pass
			{
				wc::CommandBuffer& cmd = RendererContext::computeCommandBuffer;
				cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
				//rayTracingShader.Dispatch(glm::ceil((glm::vec2)window.GetSize() / glm::vec2(m_BloomComputeWorkGroupSize)));
				if (Settings::bloomEnable) {
					cmd.BindPipeline(bloomShader.getPipeline());
					uint32_t counter = 0;

					BloomBufferSettings settings;
					settings.Params = glm::vec4(Settings::BloomThreshold, Settings::BloomThreshold - Settings::BloomKnee, Settings::BloomKnee * 2.f, 0.25f / Settings::BloomKnee);
					cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);
					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
					counter++;
					cmd.Dispatch(glm::ceil(glm::vec2(m_BloomBuffers[0].image.GetSize()) / glm::vec2(m_BloomComputeWorkGroupSize)));

					settings.Mode = (int)Renderer3D::BloomMode::Downsample;
					for (uint32_t currentMip = 1; currentMip < m_BloomMipLevels; currentMip++)
					{
						glm::vec2 dispatchSize = glm::ceil((glm::vec2)m_BloomBuffers[0].image.GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize));

						// Ping 
						settings.LOD = float(currentMip - 1);
						cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

						cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
						counter++;
						cmd.Dispatch(dispatchSize);

						// Pong 
						settings.LOD = float(currentMip);
						cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

						cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
						counter++;
						cmd.Dispatch(dispatchSize);
					}

					// First Upsample		
					settings.LOD = float(m_BloomMipLevels - 2);
					settings.Mode = (int)Renderer3D::BloomMode::UpsampleFirst;
					cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
					counter++;

					cmd.Dispatch(glm::ceil((glm::vec2)m_BloomBuffers[2].image.GetMipSize(m_BloomMipLevels - 1) / glm::vec2(m_BloomComputeWorkGroupSize)));

					settings.Mode = (int)Renderer3D::BloomMode::Upsample;
					for (int currentMip = m_BloomMipLevels - 2; currentMip >= 0; currentMip--)
					{
						settings.LOD = float(currentMip);
						cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

						cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
						counter++;

						cmd.Dispatch(glm::ceil((glm::vec2)m_BloomBuffers[2].image.GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize)));
					}

				}

				compositeShader.Bind(cmd);
				cmd.Dispatch(glm::ceil((glm::vec2)window.GetSize() / glm::vec2(m_BloomComputeWorkGroupSize)));

				cmd.End();


				VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

				submit.commandBufferCount = 1;
				submit.pCommandBuffers = cmd.GetPointer();

				submit.pWaitSemaphores = RendererContext::computeSemaphore.GetPointer();
				submit.waitSemaphoreCount = 1;

				VkPipelineStageFlags computeWaitStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

				submit.pWaitDstStageMask = &computeWaitStage;

				VulkanContext::computeQueue.Submit(submit, RendererContext::computeFence);
				RendererContext::computeFence.Wait();
				RendererContext::computeFence.Reset();
			}
		}

		std::vector<std::string> consoleHistory;
		char consoleBuffer[256];
		void RenderGUI(const float& deltaTime) {
			ImGui::Begin("Screen Render", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
			ImGui::GetBackgroundDrawList()->AddImage(m_RenderTexture, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
			ImGui::End();

			if (debug_menu) {
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Debug Menu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
				ImGui::Text(std::format("FPS: {0} FrameTime: {1}", (int)(1.f / deltaTime), deltaTime).c_str());
				ImGui::Text(std::format("Position: X:{0} Y:{1} Z:{2}", p.Position.x, p.Position.y, p.Position.z).c_str());
				ImGui::Text(std::format("Camera position: X:{0} Y:{1} Z:{2}", camera.Position.x, camera.Position.y, camera.Position.z).c_str());
				ImGui::End();
			}


			if (console) {
				//input
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, 55));
				ImGui::Begin("Console Log", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
				if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
					ImGui::SetKeyboardFocusHere(0);
				ImGui::InputText("Log", consoleBuffer, IM_ARRAYSIZE(consoleBuffer));
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

		bool console = false;

		void OnInput(const float& deltaTime) {
			// MENU MANAGMENT

			if (!console)
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

			if (Keyboard::getKey(Keyboard::Key::F1)) renderGUI = !renderGUI;
			if (Keyboard::getKey(Keyboard::Key::F2)) {
				//glm::ivec2 size = window.GetSize();
				//uint32_t byteSize = size.x * size.y * 4;
				//uint8_t* data = new uint8_t[byteSize];
				//finalImage.GetData(data);
				//stbi_flip_vertically_on_write(true);
				//stbi_write_png("screenshots/screenshot.png", size.x, size.y, 4, data, size.x * 4);
				//delete[] data;
				WC_INFO("TODO: Implement screenshots back");
			}
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
					Renderer3D::DrawOutlineCube(vMapCheck, glm::vec3(1.f), color);
					if (m_BlockBreakTimer.getElapsedTime() >= breakTime && startBreaking) {
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
				m_WorldLoaded = true;
			}
		}

		void SaveWorld() {
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) SaveChunk(chunkID);

			YAML::Node config;
			config["seed"] = worldNoise.GetSeed();
			config["time"] = angle;
			YAMLUtils::saveFile("worlds/" + worldName + "/world.properties", config);

			SavePlayerState(p);
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

		// LIGHT MANAGING (deprecated)
		uint32_t addLight(const glm::vec3& position, const uint32_t& color) {
			uint32_t light = lights.GetCounter();
			if (light <= maxLights) 
				lights.Add(Light(position, color));
			
			return light;
		}

		void removeLight(const glm::vec3& position) {
			for (uint32_t i = 0; i < maxLights; i++)
				if (lights[i].vector == position) {
					lights.Remove(i);

					break;
				}
		}

		// WORLD/CHUNK MANAGING
		void setBlockLocal(const glm::ivec3& pos, const BlockID& blockID, const ChunkID& chunkID, const bool& playerEdited, const bool& replyToServer) {
			uint16_t x = pos.x;
			uint16_t y = pos.y;
			uint16_t z = pos.z;

			m_UpdateModels = true;
			BlockID& chunkBlockID = chunks[chunkID].data[x][y][z];
			if (chunkBlockID == blockID) return;
			glm::vec3 globalPos = glm::vec3(chunks[chunkID].position) * glm::vec3(chunkSize) + glm::vec3(pos);
			glm::ivec3 iGlobalPos = globalPos;
			if (blockID == 0) {
				if (blockData[chunkBlockID].emitLight)
					removeLight(globalPos + glm::vec3(0.5f));

				if (blockData[chunkBlockID].connectionType == ConnectionType::CUSTOM_MODEL) {
					BlockMesh& mesh = blockMeshes[blockData[chunkBlockID].meshID];

					for (uint32_t i = 0; i < chunks.size(); i++)
						if (glm::vec3(m_BlockTransforms[i]) == globalPos) {
							m_BlockTransforms.Remove(i);
							mesh.cmd.instanceCount--;
							break;
						}
					m_SortModels = true;
				}
			}
			else {
				if (blockData[blockID].emitLight)
					addLight(globalPos + glm::vec3(0.5f), convertColor(glm::vec4(1.f)));

				if (blockData[blockID].connectionType == ConnectionType::CUSTOM_MODEL) {
					BlockMesh& mesh = blockMeshes[blockData[blockID].meshID];

					m_BlockTransforms.Add(glm::vec4(globalPos, blockData[blockID].meshID));
					mesh.cmd.instanceCount++;
					m_SortModels = true;
					
					ChunkAABB aabb;
					aabb.start = mesh.start + glm::vec4(globalPos, 0.f);
					aabb.end = mesh.end + glm::vec4(globalPos, 0.f);

					//chunkBVHBuffer.SetData(&aabb, sizeof(aabb), sizeof(ChunkAABB) * m_BvhCounter);
					//m_BvhCounter++;
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
			uint32_t& indexCount = chunks[chunkID].IndexCount;
			indexCount = 0;

			glm::ivec3 chunkPos = chunk.position * glm::ivec3(chunkSize);
			ChunkAABB aabb;
			aabb.start = glm::vec4(chunkPos, 0.f);
			aabb.end = aabb.start + glm::vec4(chunkSize);

			//Vertex* vertices = (Vertex*)(globalVertices + chunkID * MaxVertexCount);
			wc::CPUBuffer<Vertex> vertices;
			vertices.Create(MaxVertexCount * sizeof(Vertex));
			vertices.Map();

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
								if (blockID != checkBlock && type == ConnectionType::CONNECT_DEFAULT) { mask[n] = blockID + 1; textureMask[n] = materialData[BlockData.materialID].albedo[d]; }
								else if ((blockID == 0 || type != ConnectionType::CONNECT_DEFAULT) && checkType == ConnectionType::CONNECT_DEFAULT) { mask[n] = checkBlock + 1; textureMask[n] = materialData[checkBlockData.materialID].albedo[d + 3]; }

								else if (blockID != checkBlock && type == ConnectionType::NO_CONNECT) { mask[n] = blockID + 1; textureMask[n] = materialData[BlockData.materialID].albedo[d]; }
								else if (blockID == 0 && checkType == ConnectionType::NO_CONNECT) { mask[n] = checkBlock + 1; textureMask[n] = materialData[checkBlockData.materialID].albedo[d + 3]; }

								else if (checkBlock == 0 && type == ConnectionType::FLUID_CONNECT) { mask[n] = blockID + 1; textureMask[n] = materialData[BlockData.materialID].albedo[d]; }
								else if (blockID == 0 && checkType == ConnectionType::FLUID_CONNECT) { mask[n] = checkBlock + 1; textureMask[n] = materialData[checkBlockData.materialID].albedo[d + 3]; }
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
								glm::vec3 corner[4] = {};

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
								if (indexCount < MaxIndexCount) {
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

									for (uint32_t i = 0; i < std::size(corner); i++) {
										Vertex vertex = Vertex(corner[i] + (glm::vec3)chunkPos, { TexCoords[i], texID }, normal, blockData[blockID].materialID);
										vertices[i + offset] = vertex;
										if (vertex.Position.x < aabb.end.x) aabb.end.x = vertex.Position.x;
										if (vertex.Position.y < aabb.end.y) aabb.end.y = vertex.Position.y;
										if (vertex.Position.z < aabb.end.z) aabb.end.z = vertex.Position.z;

										if (vertex.Position.x > aabb.start.x) aabb.start.x = vertex.Position.x;
										if (vertex.Position.y > aabb.start.y) aabb.start.y = vertex.Position.y;
										if (vertex.Position.z > aabb.start.z) aabb.start.z = vertex.Position.z;
									}

									indexCount += 6;
									offset += (uint32_t)std::size(corner);
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
			vertices.Unmap();
			Renderer3D::vertexBuffer.SetData(vertices.GetBuffer(), MaxVertexCount * sizeof(Vertex), chunkID * MaxVertexCount * sizeof(Vertex));
			vertices.Destroy();
			
			VkDrawIndexedIndirectCommand cmd;
			cmd.vertexOffset = chunkID * MaxVertexCount;
			cmd.instanceCount = 1;
			cmd.firstInstance = 0;
			cmd.firstIndex = 0;
			cmd.indexCount = indexCount;
			m_IndirectBuffer.SetData(&cmd, sizeof(cmd), sizeof(cmd) * chunkID);

			if (indexCount == 0) 
				aabb.end = aabb.start = glm::vec4(0.f);	
			else {
				//chunkBVHBuffer.SetData(&aabb, sizeof(aabb), sizeof(ChunkAABB) * m_BvhCounter);
				//m_BvhCounter++;
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

			if (chunks[chunk].neighborPos[0] != -1) if (chunks[chunks[chunk].neighborPos[0]].position != neighborXpos) { chunks[chunks[chunk].neighborPos[0]].neighborNeg[0] = -1; chunks[chunk].neighborPos[0] = -1; }
			if (chunks[chunk].neighborPos[1] != -1) if (chunks[chunks[chunk].neighborPos[1]].position != neighborYpos) { chunks[chunks[chunk].neighborPos[1]].neighborNeg[1] = -1; chunks[chunk].neighborPos[1] = -1; }
			if (chunks[chunk].neighborPos[2] != -1) if (chunks[chunks[chunk].neighborPos[2]].position != neighborZpos) { chunks[chunks[chunk].neighborPos[2]].neighborNeg[2] = -1; chunks[chunk].neighborPos[2] = -1; }
			
			if (chunks[chunk].neighborNeg[0] != -1) if (chunks[chunks[chunk].neighborNeg[0]].position != neighborXneg) { chunks[chunks[chunk].neighborNeg[0]].neighborPos[0] = -1; chunks[chunk].neighborNeg[0] = -1; }
			if (chunks[chunk].neighborNeg[1] != -1) if (chunks[chunks[chunk].neighborNeg[1]].position != neighborYneg) { chunks[chunks[chunk].neighborNeg[1]].neighborPos[1] = -1; chunks[chunk].neighborNeg[1] = -1; }
			if (chunks[chunk].neighborNeg[2] != -1) if (chunks[chunks[chunk].neighborNeg[2]].position != neighborZneg) { chunks[chunks[chunk].neighborNeg[2]].neighborPos[2] = -1; chunks[chunk].neighborNeg[2] = -1; }
			
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
		
		// Bloom
		struct BloomImage {
			std::vector<ImageView> imageViews;
			Image image;

			void Create(const uint32_t& width, const uint32_t& height, const uint32_t& mipLevels) {
				VkImageCreateInfo imgInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

				imgInfo.imageType = VK_IMAGE_TYPE_2D;

				imgInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;

				imgInfo.extent.width = width;
				imgInfo.extent.height = height;
				imgInfo.extent.depth = 1;

				imgInfo.mipLevels = mipLevels;
				imgInfo.arrayLayers = 1;
				imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imgInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

				image.Create(imgInfo);

				UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
					VkImageSubresourceRange range;
					range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					range.baseArrayLayer = 0;
					range.baseMipLevel = 0;
					range.layerCount = 1;
					range.levelCount = imgInfo.mipLevels;
					image.setLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, range);
					});

				{
					VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
					createInfo.flags = 0;
					createInfo.image = image;
					createInfo.subresourceRange.layerCount = 1;
					createInfo.subresourceRange.levelCount = imgInfo.mipLevels;
					createInfo.subresourceRange.baseMipLevel = 0;
					createInfo.subresourceRange.baseArrayLayer = 0;
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

					{ // Creating the first image view
						ImageView& imageView = imageViews.emplace_back();
						imageView.Create(createInfo);
					}

					// Create The rest
					createInfo.subresourceRange.levelCount = 1;
					for (uint32_t i = 1; i < imgInfo.mipLevels; i++)
					{
						createInfo.subresourceRange.baseMipLevel = i;
						ImageView& imageView = imageViews.emplace_back();
						imageView.Create(createInfo);
					}
				}
			}

			void Destroy() {
				for (auto& view : imageViews) view.Destroy();
				image.Destroy();
			}
		} m_BloomBuffers[3];

		wc::ComputeShader bloomShader;
		wc::Sampler bloomImageSampler;
		std::vector<VkDescriptorSet> bloomSets;

		uint32_t m_BloomComputeWorkGroupSize = 4; // @TODO: REMOVE!!!
		uint32_t m_BloomMipLevels = 1;

		struct BloomBufferSettings {
			glm::vec4 Params = glm::vec4(1.f); // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
			float LOD = 0.f;
			int Mode = (int)Renderer3D::BloomMode::Prefilter;
		};

		void GenerateBloomDescriptor(const ImageView& outputView, const ImageView& bloomView) {
			DescriptorSet& descriptor = bloomSets.emplace_back();
			descriptorAllocator.allocate(descriptor, bloomShader.getDescriptorLayout());

			{
				DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(0, GetDescriptorData(bloomImageSampler, outputView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				writer.Update();
			}
			{
				wc::DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(1, GetDescriptorData(bloomImageSampler, bloomView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}
			{
				wc::DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(2, GetDescriptorData(bloomImageSampler, m_BloomBuffers[2].imageViews[0], VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}
		}
	};
}