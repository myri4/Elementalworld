// Game idea
// Space ship game where you go around planets, gather resources then go and fight people an invade their spaceships, until you invde all the galaxy
// Type: strategy, fps

#ifndef WORLD_HPP
#define WORLD_HPP

#include "Chunk.hpp"
#include "Biome.hpp"
#include <Maths/Ray.hpp>
#include <Maths/Frustum.hpp>
#include <Maths/Noise.hpp>
#include "../entities/Player.hpp"
#include <wc/Model/Animator.hpp>
#include <wc/pch.hpp>
//#include "files.hpp"
#include "../Game Mechanics/LineBatcher.hpp"
#include <GUI/Button.hpp>

uint32_t heapMemory = 0;

enum class MenuMode {GAME, INVENTORY};
MenuMode mode = MenuMode::GAME;

#define MODEL1

namespace wc {

	static AssetManager assets;
	static const uint8_t RenderDistance = 16;	
	uint8_t LoadDistance;
	Block blockData[256];
	uint32_t currentTexture = 0;

	class Singleplayer {
	private:
		// Player related
		Player p;
		Camera camera;
		float MouseSensitivity = 5;
		float gravity = 20.f;

		Random numberGen;
		LineBatcher lineBatcher;

		gl::Shader chunkShader;
		gl::UniformBuffer transforms;
		gl::UniformBuffer lights;
		uint32_t currentLightID = 0;

		struct TransformData {
			glm::mat4 proj;
			glm::mat4 view;
			float dt;
			uint32_t numLights;
			alignas(16) glm::vec3 fogColor = glm::vec3(0.f);
			glm::vec3 cameraPos;
			alignas(16) glm::vec3 ambientColor = glm::vec3(0.03f);
			float u_Density = 0.001f;
			float u_Gradient = 1.5f;
		};

		gl::IndexBuffer worldIndexBuffer;
		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> chunks;
		Noise worldNoise;
		Noise temperatureNoise;
		Noise treeNoise;
		std::pair<BlockID, glm::ivec3> missingBlocks[500];
		uint32_t numberMissingBlocks = 0;
		Frustum viewFrustum;

		int8_t water_level = 0;

		Biome mountainBiome;

#ifdef MODEL
		gl::Shader modelShader;
		Animator animator;
		Animation animation;
		Model model;
#endif

		static void AddBlockScript(const char* script) {
			Block block;

			std::string conType;
			sol::state blockState;
			blockState.script_file(script);
			if (blockState["id"].valid()) block.id = blockState["id"];
			if (blockState["isCollidable"].valid()) block.isCollidable = blockState["isCollidable"];
			if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];

			if (conType == "CONNECT_DEFAULT")    block.blockConnectionType = ConnectionType::CONNECT_DEFAULT;
			else if (conType == "FLUID_CONNECT") block.blockConnectionType = ConnectionType::FLUID_CONNECT;
			else if (conType == "NO_CONNECT")    block.blockConnectionType = ConnectionType::NO_CONNECT;
			else if (conType == "X_CONNECT")     block.blockConnectionType = ConnectionType::X_CONNECT;

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
				items[currentTexture].block = block.id;
				items[currentTexture].id = currentTexture;
				currentTexture++;
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
				items[currentTexture].block = block.id;
				items[currentTexture].id = currentTexture;
				currentTexture++;
			}

			if (blockState["emitLight"].valid()) block.emitLight = blockState["emitLight"];

			blockData[block.id] = block;
		}

		void Save(const char* outFile, const glm::ivec3& start, const glm::ivec3& end) {
			int y = start.y, x = start.x, z = start.z;
			std::ofstream file(outFile);
			for (; y < end.y; y++)
				for (x = 0; x < end.y; x++)
					for (z = 0; z < end.y; z++) 
					{
						BlockID blockID = getBlock({ x,y,z });
						if(blockID) file << (int)blockID << " " << x << " " << y - start.y << " " << z << "\n";
					}
			
			file.close();
		}
	public:
		Font font;
#define NUM_LIGHTS chunkVolume
		
		struct Light {
			uint32_t color;
			alignas(16) glm::vec3 vector;
		} lighting[NUM_LIGHTS];

		void Create() {
			chunkShader.Create("shaderpacks/default/chunkShader.glsl");

			transforms.Create(nullptr, sizeof(TransformData), GL_DYNAMIC_DRAW);
			transforms.BufferRange(0, 0, sizeof(TransformData));

			lights.Create(nullptr, sizeof(lighting), GL_DYNAMIC_DRAW);
			lights.BufferRange(1, 0, sizeof(lighting));

			sol::state worldGenState;
			worldGenState.new_usertype<glm::vec2>("vec2", sol::constructors<void(), void(float, float), void(float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y);
			worldGenState.new_usertype<glm::vec3>("vec3", sol::constructors<void(), void(float, float, float), void(float)>(), "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z, "r", &glm::vec3::r, "g", &glm::vec3::g, "b", &glm::vec3::b);
			worldGenState.new_usertype<glm::vec4>("vec4", sol::constructors<void(), void(float, float, float, float), void(float)>(), "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w, "r", &glm::vec4::r, "g", &glm::vec4::g, "b", &glm::vec4::b, "a", &glm::vec4::a);

			worldGenState.new_usertype<glm::ivec2>("ivec2", sol::constructors<void(), void(int, int), void(int)>(), "x", &glm::ivec2::x, "y", &glm::ivec2::y);
			worldGenState.new_usertype<glm::ivec3>("ivec3", sol::constructors<void(), void(int, int, int), void(int)>(), "x", &glm::ivec3::x, "y", &glm::ivec3::y, "z", &glm::ivec3::z, "r", &glm::ivec3::r, "g", &glm::ivec3::g, "b", &glm::ivec3::b);
			worldGenState.new_usertype<glm::ivec4>("ivec4", sol::constructors<void(), void(int, int, int, int), void(int)>(), "x", &glm::ivec4::x, "y", &glm::ivec4::y, "z", &glm::ivec4::z, "w", &glm::ivec4::w, "r", &glm::ivec4::r, "g", &glm::ivec4::g, "b", &glm::ivec4::b, "a", &glm::ivec4::a);

			worldGenState.new_usertype<Noise>("Noise", sol::constructors<void()>(), "lacunarity", &Noise::lacunarity, "multiplier", &Noise::multiplier, "octaves", &Noise::octaves, "persistance", &Noise::persistance, "scale", &Noise::scale, "seed", &Noise::seed);
			worldGenState.new_usertype<Block>("Block", sol::constructors<void()>(), "id", &Block::id, "texture", &Block::texture, "blockConnectionType", &Block::blockConnectionType);

			worldGenState.script_file("scripts/worldGen.lua");

			if (worldGenState["noise"].valid()) worldNoise = worldGenState["noise"];
			worldNoise.scale = 1.f / worldNoise.scale;

			if (worldGenState["TempNoise"].valid()) temperatureNoise = worldGenState["TempNoise"];
			temperatureNoise.scale = 1.f / temperatureNoise.scale;

			if (worldGenState["water_level"].valid()) water_level = worldGenState["water_level"];

			//biomeNoise.lacunarity = 2;
			//biomeNoise.multiplier = 64;
			//biomeNoise.octaves = 2;
			//biomeNoise.persistance = 0.5;
			//biomeNoise.scale = 90;
			//biomeNoise.seed = 10;
			assets.Create(30, 32, 32);

			//Loading blocks

			worldGenState.set_function("AddBlockScript", &Singleplayer::AddBlockScript);
			worldGenState.open_libraries(sol::lib::base);

			worldGenState.script_file("scripts/blocks.lua");

			load("assets/textures/misc/cursor2.png", assets.textures[0]);
			load("assets/textures/misc/hotbar.png", assets.textures[1]);
			load("assets/textures/misc/hotbar_selected.png", assets.textures[2]);
			load("assets/textures/misc/hearts.png", assets.textures[3]);

			p.Position = { (RenderDistance * RenderDistance * 0.5f + RenderDistance), RenderDistance * 4 , (RenderDistance * RenderDistance * 0.5f) };
			//p.Size = {1.f, 0.8f, 1.f};

			lineBatcher.Create();

			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				//Configuring the vertex array
				chunks[chunkID].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				chunks[chunkID].chunkMeshArray.Create();
				Renderer::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				Renderer::VertexAttribPointer(1, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
				Renderer::VertexAttribPointer(2, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, type)); // type attribute
				Renderer::VertexAttribPointer(3, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, color)); // color attribute
				Renderer::VertexAttribPointer(4, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal)); // color attribute
			}
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++)
				chunks[chunkID].chunkPos = to3D(chunkID, glm::ivec3(RenderDistance));
			for (ChunkID chunk = 0; chunk < chunks.size(); chunk++) UpdateNeighbours(chunk);


			uint32_t indices[MaxFaceCount * 6];
			uint32_t ioffset = 0;
			uint32_t i = 0;
			for (; i < sizeof(indices) / sizeof(uint32_t); i += 6) {
				indices[i + 0] = 0 + ioffset;
				indices[i + 1] = 1 + ioffset;
				indices[i + 2] = 2 + ioffset;

				indices[i + 3] = 2 + ioffset;
				indices[i + 4] = 3 + ioffset;
				indices[i + 5] = 0 + ioffset;

				ioffset += 4;
			}
			worldIndexBuffer.Create(indices, sizeof(indices));

			mountainBiome.biomeTemperature = 20;
#ifdef MODEL
			modelShader.Create("shaderpacks/default/modelShader.glsl");
			model.Create("assets/models/dancing_vampire.dae");
			animation.Create("assets/models/dancing_vampire.dae", &model);
			animator.PlayAnimation(&animation);
#endif // MODEL
			numberGen.seed = worldNoise.seed;

			BACK_FACE.CalculateNormal();
			TOP_FACE.CalculateNormal();
			LEFT_FACE.CalculateNormal();
			RIGHT_FACE.CalculateNormal();
			FRONT_FACE.CalculateNormal();
			BOTTOM_FACE.CalculateNormal();

			X_FACE1.CalculateNormal();
			X_FACE2.CalculateNormal();
			p.inventory.AddItem(10, 0, 10);
			p.inventory.AddItem(8, 1, 5);
			p.inventory.AddItem(6, 2, 5);	

			treeNoise.octaves = 5;
			treeNoise.seed = numberGen.asInt();
			treeNoise.lacunarity = 1.3f;
			treeNoise.persistance = 0.2f;
			treeNoise.multiplier = 1.f;
			treeNoise.scale = 90.f;
			//setBlock({ 273, 95, 12 },2);

			//glm::ivec3 chunkSpaceEnd = (int)chunkSize - 1 + glm::ivec3(1) * (int)chunkSize;
			//glm::ivec3 chunkSpaceStart = glm::ivec3(1) * (int)chunkSize;
		}
		wc::Button b[inventorySize];

		inline void UpdateInventory(const glm::vec2& windsize, const glm::vec2& windpos, const float& deltaTime) {
			float hotbarSize = 48.f;
			float oneSixth = hotbarSize / 6.f;
			glm::vec2 hotbarStart = glm::vec2((windsize.x - inventorySize * hotbarSize) * 0.5f, 0.f); // Temp until inventory
			for (uint8_t i = 0; i < inventorySize; i++) {

				b[i].position = hotbarStart + glm::vec2((hotbarSize + 3) * i, 0.f);
				b[i].size = { hotbarSize,hotbarSize };
				auto id = 0u;
				bool isMouseOver = b[i].isMouseOver(windpos);
				if (isMouseOver) { 
					id = 1; 
					if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::LBUTTON && wc::mouseUsed) p.inventory.RemoveItem(i);
					if (wc::Mouse::isButtonPressed() == wc::Mouse::MouseButton::RBUTTON && wc::mouseUsed) p.inventory.AddItem(p.inventory.data[i].itemID, i);
				}
				Renderer2D::DrawQuad(hotbarStart + glm::vec2((hotbarSize + 3) * i, 0.f), { hotbarSize,hotbarSize }, assets.textures[1 + id]);


				uint32_t amount = p.inventory.data[i].stack_size;
				if (amount > 0)
					Renderer2D::DrawQuad(hotbarStart + glm::vec2((hotbarSize + 3) * i + 4.f, 4.f), { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[p.inventory.data[i].itemID].texture);
				if (amount > 1)
					Renderer2D::DrawText(std::to_string(amount), font, hotbarStart + glm::vec2((hotbarSize + 3) * i + 4.f, 44.f), 0.4f, glm::vec4(1.f));
			}
		}

		void OnInputInventory() {
			wc::Mouse::ShowMouse(true);
			if (currKey == GLFW_KEY_E && wc::Action == GLFW_PRESS && wc::keyPressed) 
					mode = MenuMode::GAME;					
		}

		void Update(const glm::vec2& windsize, const float& deltaTime) {			
			//camera.center(p.Position, p.rotation.x);
			glActiveTexture(GL_TEXTURE0);
			assets.Bind();
			glActiveTexture(GL_TEXTURE1);
			assets.BindNormal();
			// activate shader
			chunkShader.use();

			float Far = chunkSize * RenderDistance; // 1100

			glm::mat4 projection = glm::perspective(glm::radians(camera.FOV), windsize.x / windsize.y, 0.1f, Far);

			// camera/view transformation
			// @TODO: Fix
			TransformData data;// { projection, camera.GetViewMatrix(), deltaTime, currentLightID, glm::vec3(0.f), camera.Position, glm::vec3(0.03f), 0.001f, 1.5f };
			data.proj = projection;
			data.view = camera.GetViewMatrix();
			data.dt = deltaTime;
			data.numLights = currentLightID;
			data.cameraPos = camera.Position;
			data.fogColor = glm::vec3(0.f);
			transforms.SetData(0, sizeof(TransformData), &data);

			lights.SetData(0, sizeof(lighting), lighting);

			viewFrustum.update(projection * camera.GetViewMatrix());
			uint8_t chunkHalf = RenderDistance / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position);
			
			for (ChunkID chunk = 0; chunk < chunks.size(); chunk++){
				if (!chunks[chunk].generated) { GenerateChunkTerrain(chunk); chunks[chunk].generated = true; }	
			
			}
			for (ChunkID i = 0; i < chunks.size(); i++) {
				if (chunks[i].IndexCount > 0 && ShowChunk(i)) {
					chunks[i].chunkMeshArray.Bind();
					Renderer::DrawIndexed(chunks[i].IndexCount);
				}				

				glm::vec3 currChunkPos = chunks[i].chunkPos;
				if (currChunkPos.x < currentPlayerPos.x - chunkHalf) ResetChunk(i, glm::ivec3(currentPlayerPos.x + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > currentPlayerPos.x + chunkHalf) ResetChunk(i, glm::ivec3(currentPlayerPos.x - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				//if (currChunkPos.y < currentPlayerPos.y - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currentPlayerPos.y + chunkHalf - 1, currChunkPos.z));
				//if (currChunkPos.y > currentPlayerPos.y + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currentPlayerPos.y - chunkHalf + 1, currChunkPos.z));

				if (currChunkPos.z < currentPlayerPos.z - chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z + chunkHalf - 1));
				if (currChunkPos.z > currentPlayerPos.z + chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z - chunkHalf + 1));
				
				// Updating the chunk`s mesh
				if (chunks[i].canBeUpdated) { UpdateMesh(i); chunks[i].canBeUpdated = false; }
			}	
#ifdef MODEL
			modelShader.use();

			auto transforms = animator.GetPoseTransforms();

			for (uint32_t i = 0; i < MAX_BONE_WEIGHTS; i++) { //transforms.size()
				std::string Transform = "finalBonesMatrices[" + std::to_string(i) + "]";
				modelShader.setMat4(Transform.c_str(), transforms[i]);
			}
			// render the loaded model
			animator.UpdateAnimation(deltaTime);
			glm::mat4 Model = glm::mat4(1.f);
			Model = glm::translate(Model, { (RenderDistance * RenderDistance * 0.5f + RenderDistance), RenderDistance * 4 , (RenderDistance * RenderDistance * 0.5f) });    // translate it down so it's at the center of the scene
			//Model = glm::scale(Model, glm::vec3(1.f)); // it's a bit too big for our scene, so scale it down
			modelShader.setMat4("model", Model);
			glDisable(GL_BLEND);
			model.Draw(modelShader);
			glEnable(GL_BLEND);
#endif
			//particleSystem.OnUpdate(deltaTime);
			//particleSystem.Emit(m_Particle);
			Renderer2D::SetLineWidth(2.f);
			lineBatcher.DrawLine({ 0,0,0 }, {0, 100, 0});
			lineBatcher.DrawLine({ 16,0,0 }, { 16, 100, 0 });
			lineBatcher.Flush();
			Renderer2D::SetLineWidth(1.f);
			// GUI
			glDisable(GL_DEPTH_TEST);
			const float scale = 0.4f;			

			//Inventory
			const float hotbarSize = 48.f;
			float oneSixth = hotbarSize / 6.f;
			glm::vec2 hotbarStart = glm::vec2((windsize.x - inventorySize * hotbarSize) * 0.5f, windsize.y - hotbarSize); // Temp until inventory

			//Health
			glm::vec2 offset = glm::vec2(0.f);
			glm::vec2 healthSize = glm::vec2(20.f);
			glm::vec2 healthStart = glm::vec2(hotbarStart.x, windsize.y - hotbarSize - 2.f - healthSize.y) - (glm::vec2(3.f, -healthSize.x) + healthSize);

			for (uint8_t i = 0; i < (int8_t)p.health; i++) {
				offset += glm::vec2(3.f, -healthSize.x) + healthSize;
				Renderer2D::DrawQuad(healthStart + offset, healthSize, assets.textures[3], { 0,0 }, { 7, 7 });
			}
			
			if ((float)((int)p.health) < p.health) 
			Renderer2D::DrawQuad(healthStart + offset + healthSize + glm::vec2(3.f, -healthSize.x), { healthSize.x * 0.5f + 2.f, healthSize.y }, assets.textures[3], { 0,0 }, { 4, 7 });
			
			Renderer2D::DrawText("FPS: " + std::to_string((int)(1.f / deltaTime)) + " Frametime: " + std::to_string(deltaTime * 1000), font, { 25.f, 5.f * scale * 10.f }, scale);
			Renderer2D::DrawText("X: " + std::to_string(p.Position.x) + " Y: " + std::to_string(p.Position.y) + " Z: " + std::to_string(p.Position.z), font, { 25.f, 15.f * scale * 10.f }, scale);
			Renderer2D::DrawText("Pitch: " + std::to_string(p.rotation.x) + " Yaw: " + std::to_string(p.rotation.y), font, { 25.f, 25.f * scale * 10.f }, scale);
			Renderer2D::DrawText(
				"ChunkX: " + std::to_string(currentPlayerPos.x) +
				" ChunkY: " + std::to_string(currentPlayerPos.y) +
				" ChunkZ: " + std::to_string(currentPlayerPos.z), font, { 25.f, 35 * scale * 10.f }, scale);
			float VelY = p.velocity.y;
			if (p.flying) VelY *= 111.1111111111111f;
			Renderer2D::DrawText("Velocity: X: " + std::to_string(p.velocity.x * 111.1111111111111f) + 
								 " Y: " + std::to_string(VelY) + 
								 " Z: " + std::to_string(p.velocity.z * 111.1111111111111f), font, { 25.f, 45.f * scale * 10.f }, scale);
			Renderer2D::DrawText("Number missing blocks: " + std::to_string(numberMissingBlocks), font, { 25.f, 55.f * scale * 10.f }, scale);
			Renderer2D::DrawText("Heap Memory: " + std::to_string(heapMemory) + " bytes", font, { 25.f, 65.f * scale * 10.f}, scale);
			Renderer2D::DrawText("Number of lights: " + std::to_string(currentLightID), font, { 25.f, 75.f * scale * 10.f }, scale);
			Renderer2D::DrawText("Look at: X: " 
				+ std::to_string(m_rayEnd.x) + " Y: " 
				+ std::to_string(m_rayEnd.y) + " Z: " 
				+ std::to_string(m_rayEnd.x) + " Looking at block: " 
				+ std::to_string(getBlock(m_rayEnd)), font, { 25.f, 85.f * scale * 10.f }, scale);
			
			for (uint8_t i = 0; i < inventorySize; i++) {
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
			glm::vec2 cursorPos = glm::vec2(windsize.x - cursorSize.x, windsize.y - cursorSize.y) * 0.5f;
			Renderer2D::DrawQuad(cursorPos, cursorSize, assets.textures[0]);
			glEnable(GL_DEPTH_TEST);
		}

		Ray ray;
		glm::vec3 m_rayEnd;
		void OnInput(const glm::ivec2& windpos, const glm::ivec2& windSize, bool& HasFocus, const float& deltaTime) {

			// MENU MANAGMENT
			if (currKey == GLFW_KEY_E && wc::Action == GLFW_PRESS && wc::keyPressed) 				
					mode = MenuMode::INVENTORY;

			// GAMEPLAY
			float yaw = glm::radians(p.rotation.x);
			float yaw90 = glm::radians(p.rotation.x + 90.f);
			if (Keyboard::isKeyPressed(Keyboard::Key::W)) { // Front
				float adder = 0.f;
				if (Keyboard::isKeyPressed(Keyboard::Key::LControl)) adder = 2.f;
				else if(Keyboard::isKeyPressed(Keyboard::Key::LShift) && !p.flying) adder = -2.f;
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
			
			if (Keyboard::isKeyPressed(Keyboard::Key::C)) { camera.FOV = 10; MouseSensitivity = 18;}
			else {
				MouseSensitivity = 5;
				camera.FOV = 90;
			}

			if (currKey == GLFW_KEY_T && wc::Action == GLFW_PRESS && wc::keyPressed) p.collision = !p.collision;

			//if (Keyboard::isKeyPressed(Keyboard::Key::Y)) {
			//	ChunkID pos = getChunkID(getChunkPos(p.Position));
			//	SaveChunk(pos);
			//}
			//
			//if (Keyboard::isKeyPressed(Keyboard::Key::U)) {
			//	ChunkID pos = getChunkID(getChunkPos(p.Position));
			//	LoadChunk(getChunkPos(p.Position), pos);
			//}

			if (mouseScrolled) {
				if (scrollY < 0) p.currentSlot++;
				else p.currentSlot--;
				if (p.currentSlot < 0) p.currentSlot = inventorySize - 1;
				else if (p.currentSlot > inventorySize - 1) p.currentSlot = 0;
			}
			
			int16_t xt, yt;

			glm::ivec2 pos = wc::Mouse::GetMousePos();

			xt = (int16_t)(windpos.x + windSize.x * 0.5f);
			yt = (int16_t)(windpos.y + windSize.y * 0.5f);

			float ms = 1.f / MouseSensitivity;

			bool invertMouse = false;
			if (invertMouse) p.rotation.x += (xt - pos.x) * ms;
			else p.rotation.x -= (xt - pos.x) * ms;

			p.rotation.y += (yt - pos.y) * ms;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (p.rotation.y >  89.f)p.rotation.y =  89.f;
			else if (p.rotation.y < -89.f)p.rotation.y = -89.f;
			
			if (p.rotation.x > 360.f) p.rotation.x = 0.f;
			else if (p.rotation.x < 0.f) p.rotation.x = 360.f;
						
			// PLAYER RELATED
			p.velocity += p.acceleration;
			p.acceleration = { 0.f,0.f,0.f };

			if (!p.flying)
				p.velocity.y -= gravity * deltaTime;

			float velocityY = p.velocity.y;
			p.Position.x += p.velocity.x * deltaTime;
			collide({ p.velocity.x,0.f,0.f });
			p.Position.y += p.velocity.y * deltaTime;
			collide({ 0.f,p.velocity.y,0.f });
			p.Position.z += p.velocity.z * deltaTime;
			collide({ 0.f,0.f,p.velocity.z });

			if (!p.wasOnGround && p.m_isOnGround && velocityY < -10.f) p.health -= 1.5f;
			p.wasOnGround = p.m_isOnGround;

			camera.Position = p.Position;
			//camera.Position.y += p.Size.y;
			camera.Yaw = p.rotation.x;
			camera.Pitch = p.rotation.y;

			p.velocity.x *= 0.009f;
			p.velocity.z *= 0.009f;
			if (p.flying)
				p.velocity.y *= 0.009f;
			//////////////

			camera.UpdateCameraAngles();
			Mouse::SetMousePosition(xt, yt);
			
			wc::Mouse::ShowMouse(!HasFocus);

			bool flyingButton = currKey == GLFW_KEY_G && wc::Action == GLFW_PRESS && wc::keyPressed;

			if (flyingButton) p.flying = !p.flying;

			bool bBreak = wc::mouseButton == (int)Mouse::MouseButton::LBUTTON && wc::mouseAction == GLFW_PRESS && wc::mouseUsed;
			bool bPlace = wc::mouseButton == (int)Mouse::MouseButton::RBUTTON && wc::mouseAction == GLFW_PRESS && wc::mouseUsed;
			bool bStart = currKey == GLFW_KEY_J && wc::Action == GLFW_PRESS && wc::keyPressed;
			bool bEnd = currKey == GLFW_KEY_K && wc::Action == GLFW_PRESS && wc::keyPressed;
			bool bLoad = currKey == GLFW_KEY_P && wc::Action == GLFW_PRESS && wc::keyPressed;

			if (bLoad) {
				Save("assets/structures/tesst.txt", sStart, sEnd);
			}			

			ray.m_rayOrigin = p.Position;
			ray.m_rayDir = camera.Front;
			glm::vec3 m_rayLastPos;
			bool bShow = true;
			
			m_rayEnd = ray.m_rayOrigin;
			while (glm::distance(ray.m_rayOrigin, m_rayEnd) < 6.f) 
			{
					m_rayEnd += ray.m_rayDir * 0.5f;
					BlockID block = getBlock(m_rayEnd);
					if (block > 0 && block != 5)
					{
						if (bShow) {
						glm::vec3 blockiPos = glm::floor(m_rayEnd) - glm::vec3(0.f);
						lineBatcher.DrawLine(blockiPos, blockiPos + glm::vec3(0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos, blockiPos + glm::vec3(1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 0.f, 0.f), blockiPos + glm::vec3(1.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 1.f, 0.f), blockiPos + glm::vec3(0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));

						lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 0.f, 1.f), blockiPos + glm::vec3(0.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 0.f, 1.f), blockiPos + glm::vec3(1.f, 0.f, 1.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 0.f, 1.f), blockiPos + glm::vec3(1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 1.f, 1.f), blockiPos + glm::vec3(0.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 0.f, 1.f));

						lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 0.f, 1.f), blockiPos, glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 0.f, 1.f), blockiPos + glm::vec3(1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));

						lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 1.f, 1.f), blockiPos + glm::vec3(0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 1.f, 1.f), blockiPos + glm::vec3(1.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
						bShow = false;

							//if (bPick)
							//	p.currentSlot = getBlock(m_rayEnd);

							if (bBreak) { 
								int itemID = block - 1;
								p.inventory.AddItem(itemID, p.currentSlot);
								setBlock(floor(m_rayEnd), 0);
							}
							else if (bPlace) {
								int itemID = p.inventory.data[p.currentSlot].itemID;
								if(p.inventory.RemoveItem(p.currentSlot))
								setBlock(m_rayLastPos, items[itemID].block);
							}

							if (bStart) { sStart = getBlockPos(floor(m_rayEnd));
								WC_INFO("Start: {0} {1} {2}", sStart.x, sStart.y, sStart.z);
							}
							if (bEnd) { sEnd = floor(m_rayEnd); 
								WC_INFO("End: {0} {1} {2}", m_rayEnd.x, m_rayEnd.y, m_rayEnd.z);
							}
							break;
						}
					}					
					m_rayLastPos = m_rayEnd;
			}
		}

	private:
		//Chunk managing

		glm::ivec3 sStart;
		glm::ivec3 sEnd;

		void collide(const glm::vec3& vel) {
			if (p.collision) {
				for (int x = p.Position.x - p.Size.x; x < p.Position.x + p.Size.x; x++) 
				for (int y = p.Position.y - p.Size.y; y < p.Position.y + p.Size.y; y++) 
				for (int z = p.Position.z - p.Size.z; z < p.Position.z + p.Size.z; z++) {
					wc::BlockID blockID = getBlock({ x, y, z });
					wc::Block block = blockData[blockID];
					if (block.id != 0u && block.isCollidable) {
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
		}

		bool ShowChunk(const ChunkID& chunk) { //@TODO: Optimize
			glm::vec3 pos1 = chunks[chunk].chunkPos * chunkPosV(chunkSize);
			glm::vec3 pos = pos1;
			if (viewFrustum.isBoxInFrustum(pos)) return true;

			pos = pos1 + glm::vec3(chunkSize, 0, 0);
			if (viewFrustum.isBoxInFrustum(pos)) return true;
			pos = pos1 + glm::vec3(0, chunkSize, 0);
			if (viewFrustum.isBoxInFrustum(pos)) return true;
			pos = pos1 + glm::vec3(0, 0, chunkSize);
			if (viewFrustum.isBoxInFrustum(pos)) return true;

			pos = pos1 + glm::vec3(chunkSize, chunkSize, 0);
			if (viewFrustum.isBoxInFrustum(pos)) return true;
			pos = pos1 + glm::vec3(chunkSize, 0, chunkSize);
			if (viewFrustum.isBoxInFrustum(pos)) return true;

			pos = pos1 + glm::vec3(0, chunkSize, chunkSize);
			if (viewFrustum.isBoxInFrustum(pos)) return true;

			pos = pos1 + glm::vec3(chunkSize);
			if (viewFrustum.isBoxInFrustum(pos)) return true;

			return false;
		}

		/*void SaveChunk(const ChunkID& chunk) {
			glm::ivec3 pos = chunks[chunk].chunkPos;
			int X = pos.x;
			int Y = pos.y;
			int Z = pos.z;
			std::string filename = "r." + std::to_string(X * cs) + "." + std::to_string(Y * cs) + "." + std::to_string(Z * cs) + ".ewr";
			
			std::ofstream file(filename);
			auto data = Compress(chunk);
			for (auto& block : data) {
				BlockID blockID = block.first;
				uint16_t count = block.second;
				file << (int)blockID << " " << count << "\n";
			}
			
			file.close();
		}

		void LoadChunk(const glm::ivec3& pos, const ChunkID& chunk) {
			int X = pos.x;
			int Y = pos.y;
			int Z = pos.z;
			std::string filename = "r." + std::to_string(X * cs) + "." + std::to_string(Y * cs) + "." + std::to_string(Z * cs) + ".ewr";
			
			std::vector<std::pair<BlockID, uint16_t>> data;
			
			std::ifstream file(filename);
			
			if (file.is_open()) {
				while (!file.eof()) {
					BlockID block;
					uint16_t count;
			
					file >> block >> count;
			
					data.emplace_back(block, count);
				}
			}
			else WC_INFO("Cant find file!");
			file.close();

			Decompress(data, chunk);			
		}

		std::vector<std::pair<BlockID, uint16_t>> Compress(const ChunkID& chunk) {
			std::vector<std::pair<BlockID, uint16_t>> compressed;
			BlockID pBlockID = chunks[chunk].chunkData[0][0][0];
			uint16_t count = 0;

			for (uint8_t y = 0; y < chunkSize; y++)
			for (uint8_t z = 0; z < chunkSize; z++)
			for (uint8_t x = 0; x < chunkSize; x++) // @TODO: optimize
			{
				BlockID block = chunks[chunk].chunkData[x][y][z];
				//if (block != 0) {
					if (block == pBlockID) count++;
					else {
						compressed.emplace_back(pBlockID, count);
						pBlockID = chunks[chunk].chunkData[x][y][z];
						count = 1;
					}
				//}
			}
			compressed.emplace_back(pBlockID, count);
			return compressed;
		}
		
		void Decompress(const std::vector<std::pair<BlockID, uint16_t>>& blocks, const ChunkID& chunk)
		{
			uint16_t counter = 0;
			for (auto& block : blocks) {
				auto blockID = block.first;
				auto count = block.second;
				for (uint16_t i = 0; i < count; i++) {
					glm::ivec3 pos = to3D(counter);
					uint8_t x = pos.x;
					uint8_t y = pos.z;
					uint8_t z = pos.y;
					chunks[chunk].chunkData[x][y][z] = blockID;
					counter++;
				}
			}

			chunks[chunk].canBeUpdated = true;
		}*/

		void ResetChunk(const ChunkID& chunk, const glm::ivec3& newChunkPos) {
			chunks[chunk].chunkPos = newChunkPos;
			UpdateNeighbours(chunk);
			GenerateChunkTerrain(chunk);
			UpdateChunkMissedBlocks(chunk);
			UpdateMesh(chunk);
		}

		void UpdateNeighbours(const ChunkID& chunk) {
			chunks[chunk].neighborXpos = getChunkID(chunks[chunk].chunkPos + chunkPosV{ 1,0,0 });
			chunks[chunk].neighborYpos = getChunkID(chunks[chunk].chunkPos + chunkPosV{ 0,1,0 });
			chunks[chunk].neighborZpos = getChunkID(chunks[chunk].chunkPos + chunkPosV{ 0,0,1 });

			chunks[chunk].neighborXneg = getChunkID(chunks[chunk].chunkPos - chunkPosV{ 1,0,0 });
			chunks[chunk].neighborYneg = getChunkID(chunks[chunk].chunkPos - chunkPosV{ 0,1,0 });
			chunks[chunk].neighborZneg = getChunkID(chunks[chunk].chunkPos - chunkPosV{ 0,0,1 });

			if (chunks[chunk].neighborXpos >= 0) { chunks[chunks[chunk].neighborXpos].neighborXneg = chunk; }
			if (chunks[chunk].neighborXneg >= 0) { chunks[chunks[chunk].neighborXneg].neighborXpos = chunk; }

			if (chunks[chunk].neighborYpos >= 0) { chunks[chunks[chunk].neighborYpos].neighborYneg = chunk; }
			if (chunks[chunk].neighborYneg >= 0) { chunks[chunks[chunk].neighborYneg].neighborYpos = chunk; }

			if (chunks[chunk].neighborZpos >= 0) { chunks[chunks[chunk].neighborZpos].neighborZneg = chunk; }
			if (chunks[chunk].neighborZneg >= 0) { chunks[chunks[chunk].neighborZneg].neighborZpos = chunk; }
		}

		// VERY TEMPORARLY!!

		void GenerateTree(const int& x, const int& y, const int& z, const ChunkID& chunk) {
			const uint32_t trunkHeight = 6;

			for(int x2 = x - 2; x2 < x + 3; x2++)
				for (int y2 = y - 3 + trunkHeight - 1; y2 < y + 2 + trunkHeight - 1; y2++)
					for (int z2 = z - 2; z2 < z + 3; z2++) {
						setBlock(glm::ivec3(x2, y2, z2) + (int)chunkSize * chunks[chunk].chunkPos, 9);
					}

			for (int i = 0; i < trunkHeight; i++) {
				setBlock(glm::ivec3(x, y + i, z) + (int)chunkSize * chunks[chunk].chunkPos, 7);
			}
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {
				memset(&chunks[chunk].chunkData, 0, sizeof(chunks[chunk].chunkData));
				for (uint8_t z = 0; z < chunkSize; z++)
				for (uint8_t x = 0; x < chunkSize; x++) {
					glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].chunkPos.x * chunkSize, z + chunks[chunk].chunkPos.z * chunkSize);
					int heightMap =	(int)worldNoise.getNoiseFor(chunkSpace.x, chunkSpace.y);
					float treeGen = treeNoise.getNoiseFor(chunkSpace.x, chunkSpace.y);

					for (uint8_t y = 0; y < chunkSize; y++) {
					glm::ivec3 pos = chunks[chunk].chunkPos * chunkPosV(chunkSize) + chunkPosV(x, y, z);
					bool onGrass = true;
					if (pos.y == heightMap) {
						if (pos.y <= water_level + (int)(treeGen * 2.f)) {
							setBlock(glm::ivec3(x, y, z), 4, chunk);
							onGrass = false;
						}
						else
							setBlock(glm::ivec3(x, y, z), 1, chunk);
					}
						int dirtDepth = (int)(treeGen * 3.f) + 1;

						if (pos.y < heightMap && pos.y >= heightMap - dirtDepth) setBlock(glm::ivec3(x, y, z), 2, chunk);
						if (pos.y < heightMap - dirtDepth) {
							if(numberGen.asInt() % 57 - 3 == 11) setBlock(glm::ivec3(x, y, z), 16, chunk);
							//else if (numberGen.asInt() % 59 - 4 == 17) setBlock(glm::ivec3(x, y, z), 11, chunk);
							else setBlock(glm::ivec3(x, y, z), 3, chunk); 
						} 
						if (pos.y > heightMap && pos.y < water_level) { setBlock(glm::ivec3(x, y, z), 5, chunk); }
						// Flowers
						if (pos.y == heightMap + 1) {
							if (onGrass) {
								if (treeGen <= 0.4f && heightMap + 1 > water_level + 1) { setBlock(glm::ivec3(x, y, z), 14, chunk); }
								else if (treeGen > 0.69f && treeGen <= 0.7f && heightMap + 1 > water_level + 1) { setBlock(glm::ivec3(x, y, z), 13, chunk); }
								else if (treeGen > 0.99f && treeGen <= 1.f && heightMap + 1 > water_level + 1) { setBlock(glm::ivec3(x, y, z), 10, chunk); }
								if (treeGen <= 0.49f && treeGen > 0.48f && x % 5 == 0 && heightMap + 1 > water_level) GenerateTree(x, y, z, chunk);
							}
							if (treeGen > 0.85f && treeGen <= 0.98f && heightMap + 1 > water_level && heightMap + 1 < water_level + 2) { setBlock(glm::ivec3(x, y, z), 15, chunk); }
						}
					}
				}	
		}

		void UpdateChunkMissedBlocks(const ChunkID& chunk) {
			if (numberMissingBlocks > 0) {
				glm::ivec3 chunkSpaceStart = chunks[chunk].chunkPos * (int)chunkSize;
				glm::ivec3 chunkSpaceEnd = (int)chunkSize + chunks[chunk].chunkPos * (int)chunkSize;
				glm::ivec3 chunkP = getChunkPos(missingBlocks[chunk].second);
				for (uint32_t i = 0; i < numberMissingBlocks; i++) {
					if ((
						chunkSpaceStart.x <= missingBlocks[i].second.x &&
						chunkSpaceStart.y <= missingBlocks[i].second.y &&
						chunkSpaceStart.z <= missingBlocks[i].second.z
						)
						&&
						(
							chunkSpaceEnd.x > missingBlocks[i].second.x &&
							chunkSpaceEnd.y > missingBlocks[i].second.y &&
							chunkSpaceEnd.z > missingBlocks[i].second.z
							)
						)
					{
						setBlock(getBlockPos(missingBlocks[i].second), missingBlocks[i].first, chunk);
						numberMissingBlocks--;
						missingBlocks[i] = missingBlocks[numberMissingBlocks];
					};
				}
			}
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block, const ChunkID& chunk) {
			uint16_t x = pos.x;
			uint16_t y = pos.y;
			uint16_t z = pos.z;

			//@TODO: Fix chunk border issue
			BlockID& chunkBlock = chunks[chunk].chunkData[x][y][z];
			if (chunkBlock == block) return;
			if (block == 0 && blockData[chunkBlock].emitLight) {
				glm::vec3 testPosition = (glm::vec3)chunks[chunk].chunkPos * (float)(chunkSize)+(glm::vec3)pos + glm::vec3(0.5f);
				for (uint32_t i = 0u; i < NUM_LIGHTS; i++) 
					if (lighting[i].vector == testPosition) {
						currentLightID--;
						lighting[i] = lighting[currentLightID];
						break;
					}				
			}
			else if (blockData[block].emitLight && currentLightID <= NUM_LIGHTS) {
				lighting[currentLightID].vector = (glm::vec3)chunks[chunk].chunkPos * (float)chunkSize + (glm::vec3)pos + glm::vec3(0.5f);
				glm::vec4 col = glm::vec4(1.f, 1.f, 1.f, 1.f);
				lighting[currentLightID].color = (uint32_t)(col.r * 255.f) << 24 | (uint32_t)(col.g * 255.f) << 16 | (uint32_t)(col.b * 255.f) << 8 | (uint32_t)(col.a * 255.f);
				currentLightID++;
			}

			chunkBlock = block;
			chunks[chunk].canBeUpdated = true;
			chunks[chunk].empty = false;

			if (x == 0) { int16_t neg = chunks[chunk].neighborXneg; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (y == 0) { int16_t neg = chunks[chunk].neighborYneg; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (z == 0) { int16_t neg = chunks[chunk].neighborZneg; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }

			if (x == chunkSizeMinusOne) { int16_t Pos = chunks[chunk].neighborXpos; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (y == chunkSizeMinusOne) { int16_t Pos = chunks[chunk].neighborYpos; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (z == chunkSizeMinusOne) { int16_t Pos = chunks[chunk].neighborZpos; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block) {
			int16_t chunk = getChunkID(getChunkPos(pos));

			glm::ivec3 blockPos = getBlockPos(pos);
			if (chunk > -1) setBlock(blockPos, block, chunk);
			else { 
				missingBlocks[numberMissingBlocks] = { block, pos };
				numberMissingBlocks++;
			}
		}

		void UpdateMesh(const ChunkID& chunk) {
			//wc::Timer timer("UpdateMesh");
			uint32_t offset = 0;
			chunks[chunk].IndexCount = 0;

			gl::Vertex chunkMesh[MaxVertexCount];

			worldIndexBuffer.Bind();
			chunks[chunk].chunkMeshArray.Bind(); // 273 95 12 9

			chunkPosV chunkPos = chunks[chunk].chunkPos * chunkPosV(chunkSize);
			auto addFace = [&](const Face& face, const uint8_t& x, const uint8_t& y, const uint8_t& z, const BlockID& block, const int8_t& type, const glm::vec4& col = glm::vec4(1.f)) {
				if (chunks[chunk].IndexCount > MaxFaceCount * 6) { WC_ERROR("Memory overflow!"); return; }
				
				glm::vec3 pos = (glm::vec3(x , y, z) + glm::vec3(chunkPos)) * blockSize;

				uint32_t texture = blockData[block].texture[(uint32_t)face.texID];
				
				uint32_t color = (uint32_t)(col.r * 255.f) << 24 | (uint32_t)(col.g * 255.f) << 16 | (uint32_t)(col.b * 255.f) << 8 | (uint32_t)(col.a * 255.f);

				const uint8_t textureSizeX = 1;
				const uint8_t textureSizeY = 1;
				
				const glm::vec2 TexCoords[4] = {
					glm::vec2(0.f, 0.f),
					glm::vec2(0.f,          textureSizeY),
					glm::vec2(textureSizeX, textureSizeY),
					glm::vec2(textureSizeX, 0.f),
				};

				chunkMesh[    offset] = gl::Vertex(face.corner1 + pos, { TexCoords[0], texture }, type, color, face.normal);
				chunkMesh[1 + offset] = gl::Vertex(face.corner2 + pos, { TexCoords[1], texture }, type, color, face.normal);
				chunkMesh[2 + offset] = gl::Vertex(face.corner3 + pos, { TexCoords[2], texture }, type, color, face.normal);
				chunkMesh[3 + offset] = gl::Vertex(face.corner4 + pos, { TexCoords[3], texture }, type, color, face.normal);
				
				chunks[chunk].IndexCount += 6;
				offset += 4;
			};

			uint8_t y = 0, x = 0, z = 0;
					for (; y < chunkSize; y++)
					for (x = 0; x < chunkSize; x++)
					for (z = 0; z < chunkSize; z++)
					{
						BlockID block = chunks[chunk].chunkData[x][y][z];
						if (block > 0)
						{
							BlockID checkBlock = 0;
							auto type = blockData[block].blockConnectionType;

							if (type == ConnectionType::CONNECT_DEFAULT) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) 
								{
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborYpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, x, y, z, block, 0);
								}

								if (z + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborZpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, x, y, z, block, 0);
								}

								if (x + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborXpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, x, y, z, block, 0);
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborYneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYneg].chunkData[x][chunkSizeMinusOne][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, x, y, z, block, 0);
								}

								if (z - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborZneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZneg].chunkData[x][y][chunkSizeMinusOne];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, x, y, z, block, 0);
								}

								if (x - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborXneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXneg].chunkData[chunkSizeMinusOne][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, x, y, z, block, 0);
								}
							}

							else if (type == ConnectionType::NO_CONNECT) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(TOP_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborYpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(TOP_FACE, x, y, z, block, 0);
								}

								if (z + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(FRONT_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborZpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(FRONT_FACE, x, y, z, block, 0);
								}

								if (x + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(RIGHT_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborXpos >= 0) {
									BlockID checkBlock = chunks[chunks[chunk].neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(RIGHT_FACE, x, y, z, block, 0);
								}
								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BOTTOM_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborYneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYneg].chunkData[x][chunkSizeMinusOne][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BOTTOM_FACE, x, y, z, block, 0);
								}

								if (z - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BACK_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborZneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZneg].chunkData[x][y][chunkSizeMinusOne];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BACK_FACE, x, y, z, block, 0);
								}

								if (x - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(LEFT_FACE, x, y, z, block, 0);
								}
								else if (chunks[chunk].neighborXneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXneg].chunkData[chunkSizeMinusOne][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(LEFT_FACE, x, y, z, block, 0);
								}
							}

							else if (type == ConnectionType::FLUID_CONNECT) // Can make a fluid face
							{
								glm::vec4 color = glm::vec4(1.f);
								if (block == 5) color.a = 0.8f; // temp
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, x, y, z, block, 1, color);
								}
								else if (chunks[chunk].neighborYpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, x, y, z, block, 1, color);
								}
							}

							else if (type == ConnectionType::X_CONNECT) // Can make a fluid face
							{
								addFace(X_FACE1, x, y, z, block, 2);
								addFace(X_FACE2, x, y, z, block, 2);
							}
						}
						else { // Chunk logic
							if (y + 1 < chunkSize) {
							if (chunks[chunk].chunkData[x][y + 1][z] == 14 || chunks[chunk].chunkData[x][y + 1][z] == 10
								|| chunks[chunk].chunkData[x][y + 1][z] == 15 || chunks[chunk].chunkData[x][y + 1][z] == 13) chunks[chunk].chunkData[x][y + 1][z] = 0;
						}
}
					}
			if (chunks[chunk].IndexCount > 0) chunks[chunk].chunkMeshBuffer.SetData(0, sizeof(chunkMesh), &chunkMesh);
		}
		 
		BlockID getBlock(const glm::ivec3& pos) {
			ChunkID chunk = getChunkID(getChunkPos(pos));
			glm::ivec3 blockPos = getBlockPos(pos);
			return chunks[chunk].chunkData[blockPos.x][blockPos.y][blockPos.z];
		}		

		int16_t getChunkID(const chunkPosV& pos) {
			//@TODO: improve
			for (ChunkID i = 0; i < chunks.size(); i++) {
				if (chunks[i].chunkPos == pos) 
					return i;				
			}

			return -1;
		}
	};	
}
#endif