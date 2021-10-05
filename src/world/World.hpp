// Game idea
// Space ship game where you go around planets, gather resources then go and fight people an invade their spaceships, until you invde all the galaxy
// Type: strategy, fps
#pragma once

#include "Chunk.hpp"
#include "Biome.hpp"
#include <Maths/Ray.hpp>
#include <Maths/Frustum.hpp>
#include <FastNoise/FastNoiseLite.h>
#include "../entities/Player.hpp"
#include <wc/Model/Animation.hpp>
#include <wc/pch.hpp>
//#include "files.hpp"
#include <wc/Skybox.hpp>
#include "../Game Mechanics/LineBatcher.hpp"
#include <GUI/Button.hpp>
#include <ppl.h>

#define MODEL

namespace wc {
	static const uint16_t RenderDistance = 16;	
	Block blockData[256];
	uint32_t currentTexture = 0;

	class Singleplayer {
	private:
		// Player related
		Camera camera;
		float MouseSensitivity = 5;
		float gravity = 20.f;

		Random numberGen;
		LineBatcher lineBatcher;

		float Far = 1100.f; // 1100
		gl::Skybox skybox;
		Frustum viewFrustum;
		gl::Shader chunkShader;
		gl::UniformBuffer transforms;
		gl::UniformBuffer lights;
		uint32_t currentLightID = 0;

		struct TransformData {
			glm::mat4 proj = glm::mat4(1.f);
			glm::mat4 view = glm::mat4(1.f);
			float dt = 0.f;
			uint32_t numLights = 0;
			alignas(16) glm::vec3 fogColor = glm::vec3(0.f);
			glm::vec3 cameraPos = glm::vec3(0.f);
			alignas(16) glm::vec3 ambientColor = glm::vec3(0.03f);
			float u_Density = 0.007f;
			float u_Gradient = 1.5f;
		};

		gl::IndexBuffer worldIndexBuffer;
		gl::VertexArray chunkMeshArray;
		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> chunks;
		FastNoiseLite worldNoise;
		FastNoiseLite temperatureNoise;
		FastNoiseLite treeNoise;
		FastNoiseLite caveNoise;
		std::array<Biome, 3> biomeMap;
		//std::pair<BlockID, glm::ivec3> missingBlocks[1000];
		//uint32_t numberMissingBlocks = 0;

		int8_t water_level = 0;

#ifdef MODEL
		gl::Shader modelShader;
		Animation animation;
		Model model;
#endif

		static void AddBlockScript(const char* script) {
			Block block;

			std::string conType;
			sol::state blockState;
			blockState.script_file(script);
			BlockID blockID = 0;
			if (blockState["id"].valid()) blockID = blockState["id"];
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
				items[currentTexture].block = blockID;
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
				items[currentTexture].block = blockID;
				items[currentTexture].id = currentTexture;
				currentTexture++;
			}

			if (blockState["emitLight"].valid()) block.emitLight = blockState["emitLight"];

			blockData[blockID] = block;
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
					setBlock(pos + offset, block);
				}
			}
		}

	public:
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
			//worldGenState.new_usertype<glm::vec2>("vec2", sol::constructors<void(), void(float, float), void(float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y);
			//worldGenState.new_usertype<glm::vec3>("vec3", sol::constructors<void(), void(float, float, float), void(float)>(), "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z, "r", &glm::vec3::r, "g", &glm::vec3::g, "b", &glm::vec3::b);
			//worldGenState.new_usertype<glm::vec4>("vec4", sol::constructors<void(), void(float, float, float, float), void(float)>(), "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w, "r", &glm::vec4::r, "g", &glm::vec4::g, "b", &glm::vec4::b, "a", &glm::vec4::a);
			//
			//worldGenState.new_usertype<glm::ivec2>("ivec2", sol::constructors<void(), void(int, int), void(int)>(), "x", &glm::ivec2::x, "y", &glm::ivec2::y);
			//worldGenState.new_usertype<glm::ivec3>("ivec3", sol::constructors<void(), void(int, int, int), void(int)>(), "x", &glm::ivec3::x, "y", &glm::ivec3::y, "z", &glm::ivec3::z, "r", &glm::ivec3::r, "g", &glm::ivec3::g, "b", &glm::ivec3::b);
			//worldGenState.new_usertype<glm::ivec4>("ivec4", sol::constructors<void(), void(int, int, int, int), void(int)>(), "x", &glm::ivec4::x, "y", &glm::ivec4::y, "z", &glm::ivec4::z, "w", &glm::ivec4::w, "r", &glm::ivec4::r, "g", &glm::ivec4::g, "b", &glm::ivec4::b, "a", &glm::ivec4::a);

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
			worldGenState.new_usertype<Block>("Block", sol::constructors<void()>(), "texture", &Block::texture, "ConnectionType", &Block::blockConnectionType);
			//worldGenState.new_enum("FractalType", {{ "None", FastNoiseLite::FractalType::FractalType_None },{ "None", FastNoiseLite::FractalType::FractalType_None } });
			worldGenState.script_file("scripts/worldGen.lua");
			if (worldGenState["noise"].valid()) worldNoise = worldGenState["noise"];
			if (worldGenState["TempNoise"].valid()) temperatureNoise = worldGenState["TempNoise"];
			if (worldGenState["TreeNoise"].valid()) treeNoise = worldGenState["TreeNoise"];
			if (worldGenState["CaveNoise"].valid()) caveNoise = worldGenState["CaveNoise"];

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
			//worldGenState.open_libraries(sol::lib::base);

			worldGenState.script_file("scripts/blocks.lua");
			assets.Free();

			load("assets/textures/misc/cursor2.png", assets.textures[0]);
			load("assets/textures/misc/hotbar.png", assets.textures[1]);
			load("assets/textures/misc/hotbar_selected.png", assets.textures[2]);
			load("assets/textures/misc/hearts.png", assets.textures[3]);

			p.Position = { (RenderDistance * RenderDistance * 0.5f), (RenderDistance * RenderDistance * 0.5f), (RenderDistance * RenderDistance * 0.5f) };

			lineBatcher.Create();

			uint32_t indices[MaxFaceCount * 6];
			uint32_t ioffset = 0;
			uint32_t i = 0;
			for (; i < ARRAYSIZE(indices); i += 6) {
				indices[i + 0] = 0 + ioffset;
				indices[i + 1] = 1 + ioffset;
				indices[i + 2] = 2 + ioffset;

				indices[i + 3] = 2 + ioffset;
				indices[i + 4] = 3 + ioffset;
				indices[i + 5] = 0 + ioffset;

				ioffset += 4;
			}
			worldIndexBuffer.Create(indices, sizeof(indices), 0);
			chunkMeshArray.Create();
			chunkMeshArray.VertexAttribPointer(0, 3, offsetof(Vertex, Position));  // position attribute
			chunkMeshArray.VertexAttribPointer(1, 3, offsetof(Vertex, TexCoords)); // texture coord attribute
			chunkMeshArray.VertexAttribPointer(2, 4, offsetof(Vertex, NormalType)); // type attribute
			chunkMeshArray.VertexAttribPointer(3, 1, offsetof(Vertex, color)); // color attribute
			WC_INFO(MaxVertexCount * sizeof(Vertex));
			chunkMeshArray.AddIndexBuffer(worldIndexBuffer);
			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				//Configuring the vertex array
				chunks[chunkID].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(Vertex), GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);

				chunks[chunkID].chunkPos = to3D(chunkID, glm::ivec3(RenderDistance));
				UpdateNeighbours(chunkID);
			}

			skybox.Create("scripts/skybox.lua", Far);
#ifdef MODEL
			modelShader.Create("shaderpacks/default/modelShader.glsl");
			model.Create("assets/models/dancing_vampire.dae");
			animation.Create("assets/models/dancing_vampire.dae", model);
#endif // MODEL
			numberGen.seed = worldNoise.GetSeed();

			BACK_FACE.CalculateNormal();
			TOP_FACE.CalculateNormal();
			LEFT_FACE.CalculateNormal();
			RIGHT_FACE.CalculateNormal();
			FRONT_FACE.CalculateNormal();
			BOTTOM_FACE.CalculateNormal();

			X_FACE1.CalculateNormal();
			X_FACE2.CalculateNormal();
			p.inventory.AddItem(10, 0, 50);
			p.inventory.AddItem(18, 1, 50);

			biomeMap[DESERT].topBlock = 4;
			biomeMap[DESERT].trees = false;
			biomeMap[DESERT].minTemp = 0.21f;
			biomeMap[DESERT].maxTemp = 0.6f;
			biomeMap[PLAINS].trees = true;
			biomeMap[PLAINS].topBlock = 1;
			biomeMap[PLAINS].minTemp = 0.61f;
			biomeMap[PLAINS].maxTemp = 1.f;
			biomeMap[SNOW_PEAK].trees = false;
			biomeMap[SNOW_PEAK].topBlock = 8;
			biomeMap[SNOW_PEAK].minTemp = -1.f;
			biomeMap[SNOW_PEAK].maxTemp = 0.2f;
			addLight(glm::vec3(0.f), convertColor(glm::vec4(1.f, 1.f, 1.f, 0.f)));
			addLight(modelPos, convertColor(glm::vec4(1.f)));
			BlockMeshes.Load("assets/models/seagrass.ply");
		}
		
		glm::vec3 modelPos = { (RenderDistance * RenderDistance * 0.5f + RenderDistance), 51.f , (RenderDistance * RenderDistance * 0.5f) };
		void Update(const glm::vec2& windsize, const float& deltaTime) {
			//camera.center(p.Position, p.rotation.x);
			assets.Bind(0);
			//assets.BindNormal(1);
			// activate shader
			chunkShader.use();

			glm::mat4 projection = glm::perspective(glm::radians(camera.FOV), windsize.x / windsize.y, 0.1f, Far);

			// camera/view transformation
			TransformData data;
			data.proj = projection;
			data.view = camera.GetViewMatrix();
			data.dt = deltaTime;
			data.numLights = currentLightID;
			data.cameraPos = camera.Position;
			skybox.Update(deltaTime, data.fogColor);
			lighting[0].vector = -glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * glm::rotate(glm::mat4(1.f), glm::radians(-skybox.angle), glm::vec3(0.f, 0.f, 1.f)));

			transforms.SetData(0, sizeof(TransformData), &data);

			lights.SetData(0, sizeof(lighting), lighting);

			viewFrustum.update(projection * camera.GetViewMatrix());
			uint8_t chunkHalf = RenderDistance / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position);

			for (ChunkID i = 0; i < chunks.size(); i++) {
				glm::vec3 currChunkPos = chunks[i].chunkPos;
				if (currChunkPos.x < currentPlayerPos.x - chunkHalf) ResetChunk(i, glm::ivec3(currentPlayerPos.x + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > currentPlayerPos.x + chunkHalf) ResetChunk(i, glm::ivec3(currentPlayerPos.x - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				if (currChunkPos.y < currentPlayerPos.y - chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currentPlayerPos.y + chunkHalf - 1, currChunkPos.z));
				if (currChunkPos.y > currentPlayerPos.y + chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currentPlayerPos.y - chunkHalf + 1, currChunkPos.z));

				if (currChunkPos.z < currentPlayerPos.z - chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z + chunkHalf - 1));
				if (currChunkPos.z > currentPlayerPos.z + chunkHalf) ResetChunk(i, glm::ivec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z - chunkHalf + 1));
			}

			for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
				if (!chunks[chunk].generated) { GenerateChunkTerrain(chunk); chunks[chunk].generated = true; }		

			for (ChunkID chunk = 0; chunk < chunks.size(); chunk++)
				if (!chunks[chunk].generatedStructures) { GenerateChunkStructures(chunk); chunks[chunk].generatedStructures = true; }

			for (ChunkID i = 0; i < chunks.size(); i++) {
			
				if (chunks[i].canBeUpdated && !chunks[i].empty) { UpdateMesh(i); chunks[i].canBeUpdated = false; }
			
				if (!chunks[i].empty && ShowChunk(i) && chunks[i].IndexCount > 0) {
					chunkMeshArray.AddVertexBuffer(chunks[i].chunkMeshBuffer, sizeof(Vertex));
					chunkMeshArray.Bind();
					Renderer::DrawIndexed(chunks[i].IndexCount);
				}
			}	
			DrawOtlineCube(sStart, sEnd - sStart, glm::vec4(1.f));
			lineBatcher.Flush();
#ifdef MODEL
			modelShader.use();

			modelShader.setMat4Array(1, animation.GetPoseTransforms(), MAX_BONE_WEIGHTS);
			
			// render the loaded model
			animation.Update(deltaTime);
			glm::mat4 Model = glm::mat4(1.f);
			Model = glm::translate(Model, modelPos);    // translate it down so it's at the center of the scene
			Model = glm::scale(Model, glm::vec3(0.3f));
			modelShader.setMat4(0, Model);
			glDisable(GL_BLEND);
			model.Draw();
			glEnable(GL_BLEND);
#endif

			skybox.Draw();
			//particleSystem.OnUpdate(deltaTime);
			//particleSystem.Emit(m_Particle);
			// GUI
			glDisable(GL_DEPTH_TEST);
			const float scale = 0.35f;			

			//Inventory
			const float hotbarSize = 48.f;
			float oneSixth = hotbarSize / 6.f;
			glm::vec2 hotbarStart = glm::vec2((windsize.x - inventorySizeX * hotbarSize) * 0.5f, windsize.y - hotbarSize); // Temp until inventory

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
			if (p.flying) VelY /= 9.f;
			Renderer2D::DrawText("Velocity: X: " + std::to_string(p.velocity.x / 9.f) + 
								 " Y: " + std::to_string(VelY) + 
								 " Z: " + std::to_string(p.velocity.z / 9.f), font, { 25.f, 45.f * scale * 10.f }, scale);
			Renderer2D::DrawText("Time of the day: " + std::to_string(skybox.angle / 6.f * 144.f), font, { 25.f, 55.f * scale * 10.f }, scale);
			//Renderer2D::DrawText("Heap Memory: " + std::to_string(modelScale) + " bytes", font, { 25.f, 65.f * scale * 10.f}, scale);
			Renderer2D::DrawText("Number of lights: " + std::to_string(currentLightID), font, { 25.f, 75.f * scale * 10.f }, scale);
			Renderer2D::DrawText("Look at: X: " 
				+ std::to_string((int)floor(m_rayEnd.x)) + " Y: " 
				+ std::to_string((int)floor(m_rayEnd.y)) + " Z: " 
				+ std::to_string((int)floor(m_rayEnd.z)) + " Looking at block: " 
				+ std::to_string(getBlock(m_rayEnd)), font, { 25.f, 85.f * scale * 10.f }, scale);
			std::string dir = "IDK";
			uint32_t Dir = (uint32_t)glm::floor(p.rotation.x) % 360;
			if (Dir == 0) dir = "Sever";
			if (Dir == 1) dir = "Ug";
			if (Dir == 2) dir = "Iztog";
			if (Dir == 3) dir = "Zapad";
			Renderer2D::DrawText("Direction: " + dir, font, { 25.f, 95.f * scale * 10.f }, scale);
			
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
			glm::vec2 cursorPos = glm::vec2(windsize.x - cursorSize.x, windsize.y - cursorSize.y) * 0.5f;
			Renderer2D::DrawQuad(cursorPos, cursorSize, assets.textures[0]);
			glEnable(GL_DEPTH_TEST);
		}

		Ray ray;
		glm::vec3 m_rayEnd;
		glm::ivec3 sStart;
		glm::ivec3 sEnd;
		//float modelScale = 0.3000f;

		void OnInput(const glm::ivec2& windpos, const glm::ivec2& windSize, bool& HasFocus, const float& deltaTime) {

			// MENU MANAGMENT
			if (Keyboard::isKeyPressed(Keyboard::Key::E) && Action == GLFW_PRESS && keyPressed)
					mode = MenuMode::INVENTORY;

			// GAMEPLAY
			float yaw = glm::radians(p.rotation.x);
			float yaw90 = glm::radians(p.rotation.x + 90.f);
			float addFOV = 0.f;
			if (Keyboard::isKeyPressed(Keyboard::Key::W)) { // Front
				float adder = 0.f;
				if (Keyboard::isKeyPressed(Keyboard::Key::LControl)) { adder = 2.f; /*addFOV = 10.f;*/ }
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
			
			if (Keyboard::isKeyPressed(Keyboard::Key::C)) { addFOV = -80.f; MouseSensitivity = 18;}
			else 
				MouseSensitivity = 5;
			
			camera.FOV = addFOV + 90.f;

			if (Keyboard::isKeyPressed(Keyboard::Key::T) && Action == GLFW_PRESS && keyPressed) p.collision = !p.collision;

			if (mouseScrolled) {
				if (scrollY < 0) p.currentSlot++;
				else p.currentSlot--;
				if (p.currentSlot < 0) p.currentSlot = inventorySizeX - 1;
				else if (p.currentSlot > inventorySizeX - 1) p.currentSlot = 0;
			}
			
			int16_t xt, yt;

			glm::ivec2 pos = Mouse::GetMousePos();

			xt = (int16_t)(windpos.x + windSize.x * 0.5f);
			yt = (int16_t)(windpos.y + windSize.y * 0.5f);

			float ms = 1.f / MouseSensitivity;

			bool invertMouse = false;
			if (invertMouse) p.rotation.x += (xt - pos.x) * ms;
			else p.rotation.x -= (xt - pos.x) * ms;

			p.rotation.y += (yt - pos.y) * ms;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (p.rotation.y > 89.f)p.rotation.y =  89.f;
			else if (p.rotation.y < -89.f)p.rotation.y = -89.f;
			
			if (p.rotation.x > 360.f) p.rotation.x = 0.f;
			else if (p.rotation.x < 0.f) p.rotation.x = 360.f;
						
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

			if (!p.wasOnGround && p.m_isOnGround && velocityY < -10.f) p.health -= 1.5f;
			p.wasOnGround = p.m_isOnGround;

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
			Mouse::SetMousePosition(xt, yt);
			
			Mouse::ShowMouse(!HasFocus);

			bool flyingButton = Keyboard::isKeyPressed(Keyboard::Key::G) && Action == GLFW_PRESS && keyPressed;

			if (flyingButton) p.flying = !p.flying;

			bool bBreak = mouseButton == (int)Mouse::MouseButton::LBUTTON && mouseAction == GLFW_PRESS && mouseUsed;
			bool bPlace = mouseButton == (int)Mouse::MouseButton::RBUTTON && mouseAction == GLFW_PRESS && mouseUsed;
			bool bSave  = Keyboard::isKeyPressed(Keyboard::Key::P) && Action == GLFW_PRESS && keyPressed;
			bool bLoad  = Keyboard::isKeyPressed(Keyboard::Key::O) && Action == GLFW_PRESS && keyPressed;
			bool bReset = Keyboard::isKeyPressed(Keyboard::Key::U) && Action == GLFW_PRESS && keyPressed;
			bool bStart = Keyboard::isKeyPressed(Keyboard::Key::V) && Action == GLFW_PRESS && keyPressed;
			bool bEnd   = Keyboard::isKeyPressed(Keyboard::Key::B) && Action == GLFW_PRESS && keyPressed;
			bool bMove  = Keyboard::isKeyPressed(Keyboard::Key::N) && Action == GLFW_PRESS && keyPressed;

			if (bReset) skybox.angle = 330.f;
			if (bMove) modelPos = p.Position;

			if (bStart) sStart = (glm::ivec3)m_rayEnd + glm::ivec3(1);
			if (bEnd) sEnd = (glm::ivec3)m_rayEnd + glm::ivec3(1);

			if (bSave) SaveStructure("assets/structures/tesst.txt", sStart, sEnd);
			if (bLoad) LoadStructure("assets/structures/tesst.txt", (glm::ivec3)p.Position);

			ray.m_rayOrigin = camera.Position;
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
						DrawOtlineCube(glm::floor(m_rayEnd), glm::vec3(1.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
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
							if (p.inventory.RemoveItem(p.currentSlot))
								setBlock(m_rayLastPos, items[itemID].block);
						}
						break;
					}
				}
				m_rayLastPos = m_rayEnd;
			}			
		}

	private:
		//Chunk managing

		void collide(const glm::vec3& vel) {
			for (int x = (int)(p.Position.x - p.Size.x); x < p.Position.x + p.Size.x; x++) 
			for (int y = (int)(p.Position.y - p.Size.y); y < p.Position.y + p.Size.y; y++) 
			for (int z = (int)(p.Position.z - p.Size.z); z < p.Position.z + p.Size.z; z++) {
				BlockID blockID = getBlock({ x, y, z });
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

			//uint8_t y = 0, x = 0, z = 0;
			//for (; y < chunkSize; y++)
			//	for (x = 0; x < chunkSize; x++)
			//		for (z = 0; z < chunkSize; z++) {
			//			if (blockData[chunks[chunk].chunkData[x][y][z]].emitLight) {
			//				glm::vec3 testPosition = (glm::vec3)chunks[chunk].chunkPos * (float)(chunkSize) + glm::vec3(x, y, z) + glm::vec3(0.5f);
			//				for (uint32_t i = 0u; i < NUM_LIGHTS; i++)
			//					if (lighting[i].vector == testPosition) {
			//						WC_INFO("Found");
			//						currentLightID--;
			//						lighting[i] = lighting[currentLightID];
			//						//break;
			//					}
			//			}
			//		}
			//GenerateChunkTerrain(chunk);
			chunks[chunk].generated = false;
			chunks[chunk].generatedStructures = false;
			chunks[chunk].canBeUpdated = true; //@TODO Redo empty check
		}

		void UpdateNeighbours(const ChunkID& chunk) {
			glm::ivec3 neighborXpos = chunks[chunk].chunkPos + chunkPosV{ 1,0,0 };
			glm::ivec3 neighborYpos = chunks[chunk].chunkPos + chunkPosV{ 0,1,0 };
			glm::ivec3 neighborZpos = chunks[chunk].chunkPos + chunkPosV{ 0,0,1 };

			glm::ivec3 neighborXneg = chunks[chunk].chunkPos - chunkPosV{ 1,0,0 };
			glm::ivec3 neighborYneg = chunks[chunk].chunkPos - chunkPosV{ 0,1,0 };
			glm::ivec3 neighborZneg = chunks[chunk].chunkPos - chunkPosV{ 0,0,1 };

			chunks[chunk].neighborXpos = -1;
			chunks[chunk].neighborYpos = -1;
			chunks[chunk].neighborZpos = -1;

			chunks[chunk].neighborXneg = -1;
			chunks[chunk].neighborYneg = -1;
			chunks[chunk].neighborZneg = -1;

			concurrency::parallel_for(ChunkID(0), (ChunkID)chunks.size(), [&](ChunkID i) {
				if (chunks[i].chunkPos == neighborXpos) { chunks[chunk].neighborXpos = i; chunks[chunks[chunk].neighborXpos].neighborXneg = chunk; }
				if (chunks[i].chunkPos == neighborYpos) { chunks[chunk].neighborYpos = i; chunks[chunks[chunk].neighborYpos].neighborYneg = chunk; }
				if (chunks[i].chunkPos == neighborZpos) { chunks[chunk].neighborZpos = i; chunks[chunks[chunk].neighborZpos].neighborZneg = chunk; }

				if (chunks[i].chunkPos == neighborXneg) { chunks[chunk].neighborXneg = i; chunks[chunks[chunk].neighborXneg].neighborXpos = chunk; }
				if (chunks[i].chunkPos == neighborYneg) { chunks[chunk].neighborYneg = i; chunks[chunks[chunk].neighborYneg].neighborYpos = chunk; }
				if (chunks[i].chunkPos == neighborZneg) { chunks[chunk].neighborZneg = i; chunks[chunks[chunk].neighborZneg].neighborZpos = chunk; }

				if (chunks[chunk].neighborXpos != -1 && chunks[chunk].neighborYpos != -1 && chunks[chunk].neighborZpos != -1 &&
					chunks[chunk].neighborXneg != -1 && chunks[chunk].neighborYneg != -1 && chunks[chunk].neighborZneg != -1) return;			
			});
		}

		// VERY TEMPORARLY!!

		void GenerateTree(const int& x, const int& y, const int& z, const float& treeHeight, const ChunkID& chunk) {
			int32_t trunkHeight = (int32_t)(treeHeight * 2.f) + 6;
			
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
			const float temperature_loss = 0.00003f;
				memset(&chunks[chunk].chunkData, 0, sizeof(chunks[chunk].chunkData));
				concurrency::parallel_for(ChunkID(0), (ChunkID)chunkSize, [&](ChunkID z){
					for (uint8_t x = 0; x < chunkSize; x++) {
						glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].chunkPos.x * chunkSize, z + chunks[chunk].chunkPos.z * chunkSize);
						int heightMap =	(int)worldNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
						float floraGen = treeNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
						float baseTemperature = 0.f;// temperatureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
						float moisture = 0.f;
						int dirtDepth = (int)(floraGen * 3.f) + 2;

						for (uint8_t y = 0; y < chunkSize; y++) {
								glm::ivec3 pos = chunks[chunk].chunkPos * chunkPosV(chunkSize) + chunkPosV(x, y, z);
								float temperature = 1.f - (pos.y * temperature_loss + baseTemperature);
								//temperature *= (1.f - (pos.y / worldNoise.GetMultiplier()));
								temperature = glm::mix(temperature, 1.f, (pos.y / worldNoise.GetMultiplier()));
								uint32_t type = getBiome(temperature, moisture);
								//float noise = -pos.y + worldNoise.get3DNoiseFor(pos.x, pos.y, pos.z);
								//if (noise < 10.f && noise > -0.1f) setBlock({x,y,z}, 3, chunk);
								float CaveNoise = caveNoise.GetNoise((float)pos.x, (float)pos.y, (float)pos.z);
								if (!(CaveNoise >= 0.25f && CaveNoise <= 0.99f)) {
									bool onSand = (pos.y <= water_level + (int)(floraGen * 3.f) && pos.y == heightMap);
									if (pos.y == heightMap) {
										if (onSand)
											setBlock(glm::ivec3(x, y, z), 4, chunk);
										else
											setBlock(glm::ivec3(x, y, z), biomeMap[type].topBlock, chunk);
									}

									if (pos.y < heightMap && pos.y >= heightMap - dirtDepth) setBlock(glm::ivec3(x, y, z), 2, chunk);
									if (pos.y < heightMap - dirtDepth) {
										//if(numberGen.asInt() % 57 - 3 == 11) setBlock(glm::ivec3(x, y, z), 16, chunk);
										//else if (numberGen.asInt() % 59 - 4 == 17) setBlock(glm::ivec3(x, y, z), 11, chunk);
										setBlock(glm::ivec3(x, y, z), 3, chunk);
									}
									if (pos.y > heightMap && pos.y < water_level) {
										setBlock(glm::ivec3(x, y, z), 5, chunk);
										if (floraGen > 0.69f && floraGen <= 0.7f && pos.y == heightMap + 1 && heightMap + 1 < water_level - 1) { setBlock(glm::ivec3(x, y, z), 17, chunk); }
										if (floraGen > 0.59f && floraGen <= 0.6f && pos.y == heightMap + 1 && heightMap + 1 < water_level - 1) { setBlock(glm::ivec3(x, y, z), 18, chunk); }
									}
									// Flowers
									if (pos.y == heightMap + 1 && heightMap + 1 > water_level) {
										if (!onSand) {
											if (floraGen <= 0.4f && heightMap > water_level) { setBlock(glm::ivec3(x, y, z), 14, chunk); }
											if (floraGen > 0.69f && floraGen <= 0.7f && heightMap > water_level) { setBlock(glm::ivec3(x, y, z), 13, chunk); }
											if (floraGen > 0.59f && floraGen <= 0.6f && heightMap > water_level) { setBlock(glm::ivec3(x, y, z), 10, chunk); }
										}
										if (floraGen > 0.95f && floraGen <= 0.98f && onSand) { setBlock(glm::ivec3(x, y, z), 15, chunk); }
									}
								}
						}
					}
					return;
				});
			//UpdateChunkMissedBlocks
			//if (numberMissingBlocks > 0) {
			//	glm::ivec3 chunkSpaceStart = chunks[chunk].chunkPos * (int)chunkSize;
			//	glm::ivec3 chunkSpaceEnd = (int)chunkSize + chunks[chunk].chunkPos * (int)chunkSize;
			//	glm::ivec3 chunkP = getChunkPos(missingBlocks[chunk].second);
			//
			//	glm::vec3 currentPlayerPos = getChunkPos(p.Position);
			//	uint8_t chunkHalf = RenderDistance / 2;
			//
			//	for (uint32_t i = 0; i < numberMissingBlocks; i++) {
			//		glm::vec3 currChunkPos = chunks[i].chunkPos;
			//		if (currChunkPos.x < currentPlayerPos.x - chunkHalf) {numberMissingBlocks--; missingBlocks[i] = missingBlocks[numberMissingBlocks];}
			//		if (currChunkPos.x > currentPlayerPos.x + chunkHalf) {numberMissingBlocks--; missingBlocks[i] = missingBlocks[numberMissingBlocks];}
			//
			//		//if (currChunkPos.y < currentPlayerPos.y - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currentPlayerPos.y + chunkHalf - 1, currChunkPos.z));
			//		//if (currChunkPos.y > currentPlayerPos.y + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currentPlayerPos.y - chunkHalf + 1, currChunkPos.z));
			//
			//		if (currChunkPos.z < currentPlayerPos.z - chunkHalf) {numberMissingBlocks--; missingBlocks[i] = missingBlocks[numberMissingBlocks];}
			//		if (currChunkPos.z > currentPlayerPos.z + chunkHalf) {numberMissingBlocks--; missingBlocks[i] = missingBlocks[numberMissingBlocks];}
			//
			//		if ((
			//			chunkSpaceStart.x <= missingBlocks[i].second.x &&
			//			chunkSpaceStart.y <= missingBlocks[i].second.y &&
			//			chunkSpaceStart.z <= missingBlocks[i].second.z
			//			)
			//			&&
			//			(
			//				chunkSpaceEnd.x > missingBlocks[i].second.x &&
			//				chunkSpaceEnd.y > missingBlocks[i].second.y &&
			//				chunkSpaceEnd.z > missingBlocks[i].second.z
			//				)
			//			)
			//		{
			//			setBlock(getBlockPos(missingBlocks[i].second), missingBlocks[i].first, chunk);
			//			numberMissingBlocks--;
			//			missingBlocks[i] = missingBlocks[numberMissingBlocks];
			//		};
			//	}
			//}
		}

		void GenerateChunkStructures(const ChunkID& chunk) {
			concurrency::parallel_for(ChunkID(0), (ChunkID)chunkSize, [&](ChunkID z) {
				for (uint8_t x = 0; x < chunkSize; x++) {
					glm::ivec2 chunkSpace = glm::ivec2(x + chunks[chunk].chunkPos.x * chunkSize, z + chunks[chunk].chunkPos.z * chunkSize);
					int heightMap = (int)worldNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
					float treeGen = treeNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);
					float baseTemperature = temperatureNoise.GetNoise((float)chunkSpace.x, (float)chunkSpace.y);

					for (uint8_t y = 0; y < chunkSize; y++) {
						glm::ivec3 pos = chunks[chunk].chunkPos * chunkPosV(chunkSize) + chunkPosV(x, y, z);
						uint32_t type = getBiome(baseTemperature, 0.f);
						float CaveNoise = caveNoise.GetNoise((float)pos.x, (float)pos.y, (float)pos.z);
						bool onGrass = !(pos.y <= water_level + (int)(treeGen * 2.f) && pos.y == heightMap);

						if (pos.y == heightMap + 1 && heightMap + 1 > water_level && onGrass && !(CaveNoise >= 0.25f && CaveNoise <= 0.99f))
								if (treeGen <= 0.49f && treeGen > 0.48f && x % 5 == 0 && biomeMap[type].trees) GenerateTree(x, y, z, treeGen, chunk);
					}
				}
				return;
			});
			//UpdateChunkMissedBlocks(chunk);
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block, const ChunkID& chunk) {
			uint16_t x = pos.x;
			uint16_t y = pos.y;
			uint16_t z = pos.z;

			BlockID& chunkBlock = chunks[chunk].chunkData[x][y][z];
			if (block == 0 && blockData[chunkBlock].emitLight) 
				removeLight((glm::vec3)chunks[chunk].chunkPos * (float)(chunkSize)+(glm::vec3)pos + glm::vec3(0.5f));			
			else if (blockData[block].emitLight) 
				addLight((glm::vec3)chunks[chunk].chunkPos * (float)chunkSize + (glm::vec3)pos + glm::vec3(0.5f), convertColor(glm::vec4(0.f, 1.f, 1.f, 1.f)));			

			chunks[chunk].chunkData[x][y][z] = block;
			chunks[chunk].canBeUpdated = true;
			chunks[chunk].empty = chunks[chunk].empty && block == 0;

			if (x == 0) { int16_t neg = chunks[chunk].neighborXneg; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (y == 0) { int16_t neg = chunks[chunk].neighborYneg; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }
			if (z == 0) { int16_t neg = chunks[chunk].neighborZneg; if (neg >= 0) { chunks[neg].canBeUpdated = true; } }

			if (x == chunkSize - 1) { int16_t Pos = chunks[chunk].neighborXpos; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (y == chunkSize - 1) { int16_t Pos = chunks[chunk].neighborYpos; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
			if (z == chunkSize - 1) { int16_t Pos = chunks[chunk].neighborZpos; if (Pos >= 0) { chunks[Pos].canBeUpdated = true; } }
		}

		uint32_t addLight(const glm::vec3& position, const uint32_t& color) {
			uint32_t light = currentLightID;
			if (currentLightID <= NUM_LIGHTS) {
				lighting[currentLightID].vector = position;
				lighting[currentLightID].color = color;
				currentLightID++;
			}
			return light;
		}

		void removeLight(const glm::vec3& position) {
			for (uint32_t i = 0u; i < NUM_LIGHTS; i++)
				if (lighting[i].vector == position) {
					currentLightID--;
					lighting[i] = lighting[currentLightID];
					break;
				}
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block) {
			int16_t chunk = getChunkID(getChunkPos(pos));

			glm::ivec3 blockPos = getBlockPos(pos);
			if (chunk > -1 && chunks[chunk].generated) setBlock(blockPos, block, chunk);
			//else { 
			//	missingBlocks[numberMissingBlocks] = { block, pos };
			//	numberMissingBlocks++;
			//}
		}

		void UpdateMesh(const ChunkID& chunk) {
			uint32_t offset = 0;
			chunks[chunk].IndexCount = 0;
			Vertex vertices[4];

			chunkPosV chunkPos = chunks[chunk].chunkPos * chunkPosV(chunkSize);
			//void* ptr = chunks[chunk].chunkMeshBuffer.Map(GL_WRITE_ONLY);
			auto addFace = [&](const Face& face, const glm::vec3& pos, const BlockID& block, const int8_t& type, const uint32_t& color = 0xFFFFFF) {
				if (chunks[chunk].IndexCount >= MaxFaceCount * 6) { /*WC_ERROR("Memory overflow!");*/ return; }

				if (block == 218) {
					for (uint32_t i = 0; i < BlockMeshes.vertices.size(); i++) {
						chunks[chunk].chunkMeshBuffer.SetData(offset, sizeof(Vertex), &BlockMeshes.vertices[i]);
						offset += sizeof(Vertex);
					}
				}
				else {
				
					uint32_t& texture = blockData[block].texture[(uint32_t)face.texID];
				
					const uint8_t textureSizeX = 1;
					const uint8_t textureSizeY = 1;
				
					const glm::vec2 TexCoords[4] = {
						glm::vec2(0.f, 0.f),
						glm::vec2(0.f,          textureSizeY),
						glm::vec2(textureSizeX, textureSizeY),
						glm::vec2(textureSizeX, 0.f),
					};
				
					vertices[1] = Vertex(face.corner1 + pos, { TexCoords[0], texture }, type, color, face.normal);
					vertices[0] = Vertex(face.corner2 + pos, { TexCoords[1], texture }, type, color, face.normal);
					vertices[3] = Vertex(face.corner3 + pos, { TexCoords[2], texture }, type, color, face.normal);
					vertices[2] = Vertex(face.corner4 + pos, { TexCoords[3], texture }, type, color, face.normal);
					//memset((void*)((int*)ptr + offset), (int)vertices, sizeof(vertices));
				
					chunks[chunk].chunkMeshBuffer.SetData(offset, sizeof(vertices), vertices);
					chunks[chunk].IndexCount += 6;
					offset += sizeof(vertices);
				}
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
							glm::vec3 pos = (glm::vec3(x, y, z) + glm::vec3(chunkPos)) * blockSize;

							if (type == ConnectionType::CONNECT_DEFAULT) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) 
								{
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborYpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, pos, block, 0);
								}

								if (z + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborZpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, pos, block, 0);
								}

								if (x + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborXpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, pos, block, 0);
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborYneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYneg].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, pos, block, 0);
								}

								if (z - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborZneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZneg].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, pos, block, 0);
								}

								if (x - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborXneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXneg].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 || type != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, pos, block, 0);
								}
							}

							else if (type == ConnectionType::NO_CONNECT) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(TOP_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborYpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(TOP_FACE, pos, block, 0);
								}

								if (z + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(FRONT_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborZpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(FRONT_FACE, pos, block, 0);
								}

								if (x + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(RIGHT_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborXpos >= 0) {
									BlockID checkBlock = chunks[chunks[chunk].neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(RIGHT_FACE, pos, block, 0);
								}
								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BOTTOM_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborYneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYneg].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BOTTOM_FACE, pos, block, 0);
								}

								if (z - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BACK_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborZneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZneg].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(BACK_FACE, pos, block, 0);
								}

								if (x - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(LEFT_FACE, pos, block, 0);
								}
								else if (chunks[chunk].neighborXneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXneg].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 || (type != blockData[checkBlock].blockConnectionType && type != ConnectionType::X_CONNECT))
										addFace(LEFT_FACE, pos, block, 0);
								}
							}

							else if (type == ConnectionType::FLUID_CONNECT) // Can make a fluid face
							{
								glm::vec4 col = glm::vec4(1.f);
								if (block == 5) col.a = 0.8f;
								uint32_t color = convertColor(col);
								if (y + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, pos, block, 1, color);
								}
								else if (chunks[chunk].neighborYpos >= 0) {// temp
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, pos, block, 1, color);
								}

								if (x + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x + 1][y][z];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0) // Temp
										addFace(RIGHT_FACE, pos, block, 1, color);
								}
								else if (chunks[chunk].neighborXpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXpos].chunkData[0][y][z];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(RIGHT_FACE, pos, block, 1, color);
								}

								if (z + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y][z + 1];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(FRONT_FACE, pos, block, 1, color);
								}
								else if (chunks[chunk].neighborZpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZpos].chunkData[x][y][0];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(FRONT_FACE, pos, block, 1, color);
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y - 1][z];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(BOTTOM_FACE, pos, block, 1, color);
								}
								else if (chunks[chunk].neighborYneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYneg].chunkData[x][chunkSize - 1][z];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(BOTTOM_FACE, pos, block, 1, color);
								}

								if (z - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x][y][z - 1];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(BACK_FACE, pos, block, 1, color);
								}
								else if (chunks[chunk].neighborZneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborZneg].chunkData[x][y][chunkSize - 1];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(BACK_FACE, pos, block, 1, color);
								}

								if (x - 1 >= 0) {
									checkBlock = chunks[chunk].chunkData[x - 1][y][z];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(LEFT_FACE, pos, block, 1, color);
								}
								else if (chunks[chunk].neighborXneg >= 0) {
									checkBlock = chunks[chunks[chunk].neighborXneg].chunkData[chunkSize - 1][y][z];
									if (type != blockData[checkBlock].blockConnectionType && checkBlock == 0)
										addFace(LEFT_FACE, pos, block, 1, color);
								}
							}

							else if (type == ConnectionType::X_CONNECT) // Can make a fluid face
							{
								addFace(X_FACE1, pos, block, 2);
								addFace(X_FACE2, pos, block, 2);
							}
						}
						else { // Chunk logic
							if (y + 1 < chunkSize) {
							if (chunks[chunk].chunkData[x][y + 1][z] == 14 || chunks[chunk].chunkData[x][y + 1][z] == 10
								|| chunks[chunk].chunkData[x][y + 1][z] == 15 || chunks[chunk].chunkData[x][y + 1][z] == 13) chunks[chunk].chunkData[x][y + 1][z] = 0;
							}
						}
					}
					//chunks[chunk].chunkMeshBuffer.UnMap();
					chunks[chunk].empty = chunks[chunk].IndexCount == 0;
		}
		 
		BlockID getBlock(const glm::ivec3& pos) {
			int16_t chunk = getChunkID(getChunkPos(pos));
			glm::ivec3 blockPos = getBlockPos(pos);
			if (chunk < 0) return 0;
			return chunks[chunk].chunkData[blockPos.x][blockPos.y][blockPos.z];
		}

		int16_t getChunkID(const chunkPosV& pos) {
			for (ChunkID i = 0; i < chunks.size(); i++) {
					if (chunks[i].chunkPos == pos) 
						return i;				
			}
			return -1;
		}

		uint32_t getBiome(const float& temperature, const float& moisture = 0.f) {
			for (uint32_t i = 0; i < biomeMap.size(); i++) {
				if (temperature >= biomeMap[i].minTemp && temperature <= biomeMap[i].maxTemp) return i;
			}
			return PLAINS;
		}

		void DrawOtlineCube(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color) {
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