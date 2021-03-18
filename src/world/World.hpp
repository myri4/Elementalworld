// Game idea
// Space ship game where you go around planets, gather resources then go and fight people an invade their spaceships, until you invde all the galaxy
// Type: strategy, fps

#ifndef WORLD_HPP
#define WORLD_HPP

#include "Chunk.hpp"
#include "Block.hpp"
#include "../entityes/Player.hpp"
#include <Utils/Time.hpp>
#include <Maths/Frustum.hpp>
#include <Maths/Noise.hpp>
#include <map>
#include "Biome.hpp"
#include <GUI/AssetManager.hpp>
#include <wc/Model/Animator.hpp>
#include <GUI/Renderer2D.hpp>
#include <Utils/Bits.hpp>

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
		BlockEdit
	};

	static std::unordered_map<int, Block> blockData;
	static AssetManager assets;
	static const uint8_t RenderDistance = 16;

	static void AddBlock(const char* script) {
		Block block;

		std::string conType;
		sol::state blockState;
		blockState.script_file(script);
		if (blockState["id"].valid()) block.id = blockState["id"];
		if (blockState["isCollidable"].valid()) if (blockState["isCollidable"]) enableBit(block.flags, isCollidableFlag);
		if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];

		if (conType == "CONNECT_DEFAULT") block.blockConnectionType = ConnectionType::CONNECT_DEFAULT;
		if (conType == "FLUID_CONNECT")   block.blockConnectionType = ConnectionType::FLUID_CONNECT;
		if (conType == "NO_CONNECT")      block.blockConnectionType = ConnectionType::NO_CONNECT;
		if (conType == "X_CONNECT")      block.blockConnectionType = ConnectionType::X_CONNECT;
		
		
		if (blockState["allTextures"].valid()) {
			block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(blockState["allTextures"]);
			block.texture[(int)BlockTexture::BOTTOM] = block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::FRONT] =  block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::BACK] =   block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::LEFT] =   block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::RIGHT] =  block.texture[(int)BlockTexture::TOP];
		}
		if(blockState["top"].valid())    block.texture[(int)BlockTexture::TOP] =    assets.LoadTexture(blockState["top"]);
		if(blockState["bottom"].valid()) block.texture[(int)BlockTexture::BOTTOM] = assets.LoadTexture(blockState["bottom"]);
		if(blockState["front"].valid())  block.texture[(int)BlockTexture::FRONT] =  assets.LoadTexture(blockState["front"]);
		if(blockState["back"].valid())   block.texture[(int)BlockTexture::BACK] =   assets.LoadTexture(blockState["back"]);
		if(blockState["left"].valid())   block.texture[(int)BlockTexture::LEFT] =   assets.LoadTexture(blockState["left"]);
		if(blockState["right"].valid())  block.texture[(int)BlockTexture::RIGHT] =  assets.LoadTexture(blockState["right"]);

		if (blockState["emitLight"].valid()) if (blockState["emitLight"]) enableBit(block.flags, emitLightFlag);

		blockData[block.id] = block;
	}

	//struct quad_t
	//{
	//	glm::ivec3 a, b, c, d;
	//};

	//static size_t to_index(const uint32_t u, const uint32_t v, const uint32_t sz_u)
	//{
	//	return static_cast<size_t>(v) * static_cast<size_t>(sz_u) + static_cast<size_t>(u);
	//}

	//static auto greedy_remesher(const glm::uvec3& dims,	const std::function<bool(glm::ivec3)>& map_fn) {
	//	Face* quads;
	//	uint32_t count = 0;
	//	uint8_t norm = 0u;
	//	for (; norm < 3u; norm++) {

	//		const auto tan = (norm + 1u) % 3u;
	//		const auto biTan = (norm + 2u) % 3u;

	//		glm::ivec3 normalVector(0);
	//		normalVector[norm] = 1;

	//		std::vector<bool> mask(dims[tan] * dims[biTan]);

	//		for (size_t slice = 0u; slice <= dims[norm]; slice++) {

	//			glm::uvec3 cursor(0);
	//			cursor[norm] = slice;

	//			for (cursor[biTan] = 0; cursor[biTan] < dims[biTan]; ++cursor[biTan])
	//			{
	//				for (cursor[tan] = 0; cursor[tan] < dims[tan]; ++cursor[tan])
	//				{
	//					const glm::ivec3 curr(cursor);
	//					const glm::ivec3 vec(normalVector);

	//					const auto voxel_in_slice = map_fn(curr);
	//					const auto voxel_in_previous_slice = map_fn(curr - vec);

	//					const auto i = to_index(cursor[tan], cursor[biTan], dims[tan]);
	//					mask[i] = voxel_in_slice != voxel_in_previous_slice;
	//				}
	//			}

	//			// Generate mesh for mask using lexicographic ordering
	//			for (size_t y = 0; y < dims[biTan]; y++) {
	//				for (size_t x = 0; x < dims[tan];) {
	//					if (!mask[to_index(x, y, dims[tan])])
	//					{
	//						x++;
	//						continue;
	//					}

	//					size_t width = 1;
	//					while (x + width < dims[tan]
	//						&& mask[to_index(x + width, y, dims[tan])])
	//					{
	//						width++;
	//					}

	//					size_t height = 1;
	//					for (; y + height < dims[biTan]; height++) {
	//						for (auto k = x; k < x + width; k++) {
	//							if (!mask[to_index(k, y + height, dims[tan])]) {
	//								goto done_quad;
	//							}
	//						}
	//					}

	//				done_quad:
	//					// The base of the quad to add
	//					glm::ivec3 b(0);
	//					b[norm] = slice;
	//					b[tan] = x;
	//					b[biTan] = y;

	//					// The 'width' of the quad.
	//					glm::ivec3 du(0);
	//					du[tan] = width;

	//					// The 'height' of the quad.
	//					glm::ivec3 dv(0);
	//					dv[biTan] = height;

	//					quads[count] = { b, b + du, b + du + dv, b + dv };
	//					count++;

	//					// Clear the mask and increment x by the width of this quad.
	//					for (size_t l = 0; l < height; ++l)
	//					{
	//						for (size_t k = 0; k < width; ++k)
	//						{
	//							const auto i = to_index(x + k,
	//								y + l,
	//								dims[tan]);

	//							mask[i] = false;
	//						}
	//					}

	//					x += width;
	//				}
	//			}
	//		}
	//	}

	//	return quads;
	//}

	class Singleplayer {
	private:
		// Player related
		Player p;
		glm::mat4 projection = glm::mat4(0.0f);
		Camera camera;
		float MouseSensitivity = 5;


		gl::Shader chunkShader;

		gl::IndexBuffer worldIndexBuffer;
		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> world;
		Noise worldNoise;
		//Noise biomeNoise;
		Frustum viewFrustum;

		int8_t water_level = 0;
		//int8_t snow_level = 0;
		//int8_t currentLight = 0;

		gl::Texture cursorTex;

		std::string path;

		//gl::Shader modelShader;
		//Animator animator;
		//Animation animation;
		//Model model;
	public:
		Font font;

		Singleplayer() {}

		void Create() {
			chunkShader.Create("shaderpacks/default/chunkShader.glsl");
			sol::state luaState;
			luaState.script_file("scripts/worldGen.lua");

			worldNoise.lacunarity = luaState["lacunarity"];
			worldNoise.multiplier = luaState["multiplier"];
			worldNoise.octaves = luaState["octaves"];
			worldNoise.persistance = luaState["persistance"];
			worldNoise.scale = luaState["scale"];
			worldNoise.seed = luaState["seed"];

			water_level = luaState["water_level"];
			//snow_level = noiseState["snow_level"];

			//biomeNoise.lacunarity = 2;
			//biomeNoise.multiplier = 64;
			//biomeNoise.octaves = 2;
			//biomeNoise.persistance = 0.5;
			//biomeNoise.scale = 90;
			//biomeNoise.seed = 10;

			assets.Create(30, 32, 32);

			//luaState.new_usertype<glm::vec2>("vec2", sol::constructors<void(), void(float, float), void(float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y);
			//
			//luaState.new_usertype<glm::vec3>("vec3", sol::constructors<void(), void(float, float, float), void(float)>(), "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z,
			//	"r", &glm::vec3::r, "g", &glm::vec3::g, "b", &glm::vec3::b);
			//luaState.new_usertype<glm::vec4>("vec4", sol::constructors<void(), void(float, float, float, float), void(float)>(), "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w,
			//	"r", &glm::vec4::r, "g", &glm::vec4::g, "b", &glm::vec4::b, "a", &glm::vec4::a);
			//
			//luaState.new_usertype<Block>("Block", sol::constructors<void(), void(const char*)>(), "id", &Block::id, "isCollidable", &Block::isCollidable);

			//Loading blocks

			luaState.set_function("AddBlock", &AddBlock);
			luaState.open_libraries(sol::lib::base);

			luaState.script_file("scripts/blocks.lua");

			camera.Position = { RenderDistance * RenderDistance / 2 + RenderDistance, RenderDistance * 4 ,RenderDistance * RenderDistance / 2 };
			ChunkID chunkID = 0;
			for (; chunkID < world.size(); chunkID++) {
				//Configuring the vertex array
				world[chunkID].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				world[chunkID].chunkMeshArray.Create();
				Renderer::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				Renderer::VertexAttribPointer(1, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
				Renderer::VertexAttribPointer(3, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, type)); // type attribute
				world[chunkID].chunkPos = to3D(chunkID, glm::ivec3(RenderDistance));
			}

			{
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
			}
			
			for (ChunkID chunk = 0; chunk < world.size(); chunk++) UpdateNeighbours(chunk);

			cursorTex.load("assets/textures/misc/cursor2.png");

			//defBiome.Create("scripts/biomeTest.lua");

			//modelShader.Create("shaderpacks/default/modelShader.glsl");
			//model.Create("assets/models/dancing_vampire.dae");
			//animation.Create("assets/models/dancing_vampire.dae", &model);
			//animator.PlayAnimation(&animation);

			//uint8_t flags = canBeUpdatedFlag | usedFlag;
			//
			//disableBit(flags, canBeUpdatedFlag);
			//
			//WC_INFO(bitEnabled(flags, 0));
		}

		void Update(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {
			{
				int16_t xt, yt;

				glm::vec2 pos = wc::Mouse::GetMousePos();

				xt = windpos.x + windsize.x / 2;
				yt = windpos.y + windsize.y / 2;

				bool invertMouse = false;
				if (invertMouse) camera.Yaw += (xt - pos.x) / MouseSensitivity;
				else camera.Yaw -= (xt - pos.x) / MouseSensitivity;

				camera.Pitch += (yt - pos.y) / MouseSensitivity;

				// make sure that when pitch is out of bounds, screen doesn't get flipped
				if (camera.Pitch > 89.0f) camera.Pitch = 89.0f;
				if (camera.Pitch < -89.0f)camera.Pitch = -89.0f;

				if (camera.Yaw > 360.0f)camera.Yaw = 0.0f;
				if (camera.Yaw < 0.0f)  camera.Yaw = 360.0f;

				if (CenterMouse) {
					camera.UpdateCameraAngles();
					wc::Mouse::SetMousePosition(xt, yt);
				}

			float Far = chunkSize * RenderDistance; // 1100
			projection = glm::perspective(glm::radians(camera.FOV), windsize.x / windsize.y, 0.1f, Far);

			p.Position = camera.Position;
			}
	
			assets.Bind();
			// activate shader
			chunkShader.use();
			chunkShader.setVec3("viewPos", camera.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			chunkShader.setMat4("u_Projection", projection);

			// pass the delta time variable to the shader
			//chunkShader.setFloat("deltaTime", deltaTime);

			// camera/view transformation
			chunkShader.setMat4("u_View", camera.GetViewMatrix());

			viewFrustum.update(projection * camera.GetViewMatrix());
			uint8_t chunkHalf = RenderDistance / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position);
			for (ChunkID i = 0; i < world.size(); i++) {

				if (world[i].IndexCount > 0 && ShowChunk(i)) {
					glm::vec3 pos = world[i].chunkPos * glm::vec3(chunkSize);
					world[i].chunkMeshArray.Bind();
					chunkShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), pos)); // calculate the model matrix for each object and pass it to shader before drawing
					Renderer::DrawIndexed(world[i].IndexCount);
				}				

				glm::vec3 currChunkPos = world[i].chunkPos;
				if (currChunkPos.x < currentPlayerPos.x - chunkHalf) ResetChunk(i, glm::vec3(currentPlayerPos.x + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > currentPlayerPos.x + chunkHalf) ResetChunk(i, glm::vec3(currentPlayerPos.x - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				//if (currChunkPos.y < currentPlayerPos.y - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currentPlayerPos.y + chunkHalf - 1, currChunkPos.z));
				//if (currChunkPos.y > currentPlayerPos.y + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currentPlayerPos.y - chunkHalf + 1, currChunkPos.z));

				if (currChunkPos.z < currentPlayerPos.z - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z + chunkHalf - 1));
				if (currChunkPos.z > currentPlayerPos.z + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z - chunkHalf + 1));

				// Updating the chunk`s mesh
				if (!bitEnabled(world[i].flags, 1)) { GenerateChunkTerrain(i);	enableBit(world[i].flags, generatedFlag); }
				//if (world[i].canBeUpdated) { UpdateMesh(i);	world[i].canBeUpdated = false;	} 
				if (bitEnabled(world[i].flags, 2)) { UpdateMesh(i);	disableBit(world[i].flags, canBeUpdatedFlag); }
			}			

			//modelShader.use();
			//modelShader.setMat4("projection", p.projection);
			//modelShader.setMat4("view", p.GetView());

			//auto transforms = animator.GetPoseTransforms();

			//for (uint32_t i = 0; i < transforms.size(); i++) {
			//	std::string Transform = "finalBonesMatrices[" + std::to_string(i) + "]";
			//	modelShader.setMat4(Transform.c_str(), transforms[i]);
			//}
			// render the loaded model
			//animator.UpdateAnimation(deltaTime, modelShader);
			//glm::mat4 Model = glm::mat4(1.0f);
			//Model = glm::translate(Model, { 161, 47.5f ,121 }); // translate it down so it's at the center of the scene
			//Model = glm::scale(Model, glm::vec3(1.f));	// it's a bit too big for our scene, so scale it down
			//modelShader.setMat4("model", Model);
			//glDisable(GL_CULL_FACE);
			//glDisable(GL_BLEND);
			//model.Draw(modelShader);
			//glEnable(GL_CULL_FACE);
			//glEnable(GL_BLEND);

			// Render 2D Stuff
			Renderer2D::DrawTexts("X: " + std::to_string(p.Position.x) + " Y: " + std::to_string(p.Position.y) + " Z: " + std::to_string(p.Position.z), font, { 25.0f, 60 });
			Renderer2D::DrawTexts("Pitch: " + std::to_string(camera.Pitch) + " Yaw: " + std::to_string(camera.Yaw), font, { 25.0f, 100 });
			Renderer2D::DrawTexts(
				"ChunkX: " + std::to_string(currentPlayerPos.x) +
				" ChunkY: " + std::to_string(currentPlayerPos.y) +
				" ChunkZ: " + std::to_string(currentPlayerPos.z), font, { 25.0f, 140 });

			//Renderer2D::DrawLine({ 0,0 }, { 200, 200 }, glm::vec4(1, 0.f, 0.f, 1.f));

			{
				glm::vec2 cursorSize = cursorTex.GetSize() * glm::vec2(1.4f);
				glm::vec2 cursorPos = {windsize.x / 2 - cursorSize.x, windsize.y / 2 - cursorSize.y };
				Renderer2D::DrawQuad(cursorPos, cursorSize, cursorTex);
			}

		}

		void OnInput(const float& deltaTime) {
			{
				float velocity = p.MovementSpeed * deltaTime;
				float yaw = glm::radians(camera.Yaw);
				float yaw90 = glm::radians(camera.Yaw + 90.0f);
				if (Keyboard::isKeyPressed(Keyboard::Key::W)) { // Front
					camera.Position.x += glm::cos(yaw) * velocity;
					camera.Position.z += glm::sin(yaw) * velocity;
				}

				if (Keyboard::isKeyPressed(Keyboard::Key::S)) { // Back
					camera.Position.x -= glm::cos(yaw) * velocity;
					camera.Position.z -= glm::sin(yaw) * velocity;
				}
				if (Keyboard::isKeyPressed(Keyboard::Key::A)) { // Left
					camera.Position.x -= glm::cos(yaw90) * velocity;
					camera.Position.z -= glm::sin(yaw90) * velocity;
				}
				if (Keyboard::isKeyPressed(Keyboard::Key::D)) { // Right
					camera.Position.x += glm::cos(yaw90) * velocity;
					camera.Position.z += glm::sin(yaw90) * velocity;
				}
				if (Keyboard::isKeyPressed(Keyboard::Key::Space))  camera.Position.y += velocity;			  // Up
				if (Keyboard::isKeyPressed(Keyboard::Key::LShift)) camera.Position.y -= velocity;			  // Down
				if (Keyboard::isKeyPressed(Keyboard::Key::C)) { camera.FOV = 10; MouseSensitivity = 18; }
				else {
					MouseSensitivity = 5;
					camera.FOV = 90;
				}

				if (Keyboard::isKeyPressed(Keyboard::Key::Y)) {
					ChunkID pos = getChunkID(getChunkPos(p.Position));
					SaveChunk(pos);
				}

				if (Keyboard::isKeyPressed(Keyboard::Key::U)) {
					ChunkID pos = getChunkID(getChunkPos(p.Position));
					LoadChunk(getChunkPos(p.Position), pos);
				}

				if (Keyboard::isKeyPressed(Keyboard::Key::Num1)) p.ItemHolding = 1;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num2)) p.ItemHolding = 2;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num2)) p.ItemHolding = 2;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num3)) p.ItemHolding = 3;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num4)) p.ItemHolding = 4;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num5)) p.ItemHolding = 5;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num6)) p.ItemHolding = 6;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num7)) p.ItemHolding = 7;
				if (Keyboard::isKeyPressed(Keyboard::Key::Num8)) p.ItemHolding = 11;
			}
			bool bBreak = Mouse::isButtonPressed() == Mouse::MouseButton::LBUTTON;
			bool bPlace = Mouse::isButtonPressed() == Mouse::MouseButton::RBUTTON;

			if (bBreak || bPlace) {
				glm::vec3 m_rayLastPos = glm::vec3(0.0f);
				Ray ray(p.Position);
				while (ray.getLength() < 6) {
					ray.m_rayEnd += camera.Front * 0.5f;
					BlockID block = getBlock(ray.getEnd());
					if (block > 0 && block != 5)
					{
						if (bBreak) { 
							setBlock(glm::floor(ray.getEnd()), 0); break; }
						else if (bPlace) { setBlock(glm::floor(m_rayLastPos), p.ItemHolding); break; }
					}
					
					m_rayLastPos = ray.getEnd();
				}
			}
		}
	private:

		//Chunk managing

		bool ShowChunk(const ChunkID& chunk) { //@TODO: Optimize
			glm::vec3 pos1 = world[chunk].chunkPos * glm::vec3(chunkSize);
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

		void SaveChunk(const ChunkID& chunk) {
			glm::vec3 pos = world[chunk].chunkPos;
			int X = pos.x;
			int Y = pos.y;
			int Z = pos.z;
			std::string filename = "r." + std::to_string(X / chunkSize) + "." + std::to_string(Y / chunkSize) + "." + std::to_string(Z / chunkSize) + ".ewr";

			std::ofstream file(filename);
			auto data = Compress(chunk);
			for (auto& block : data) {
				BlockID blockID = block.first;
				uint16_t count = block.second;
				file << (int)blockID << " " << count << "\n";
			}

			file.close();
			//stbi_load
		}

		void LoadChunk(const glm::vec3& pos, const ChunkID& chunk) {
			int X = pos.x;
			int Y = pos.y;
			int Z = pos.z;
			std::string filename = "r." + std::to_string(X / chunkSize) + "." + std::to_string(Y / chunkSize) + "." + std::to_string(Z / chunkSize) + ".ewr";

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

			//Decompress(data, chunk);			
		}

		std::vector<std::pair<BlockID, uint16_t>> Compress(const ChunkID& chunk) {
			std::vector<std::pair<BlockID, uint16_t>> compressed;
			BlockID pBlockID = world[chunk].chunkData[0][0][0];
			uint16_t count = 0;

			for (uint8_t y = 0; y < chunkSize; y++)
			for (uint8_t z = 0; z < chunkSize; z++)
			for (uint8_t x = 0; x < chunkSize; x++) // @TODO: optimize
			{
				BlockID block = world[chunk].chunkData[x][y][z];
				//if (block != 0) {
					if (block == pBlockID) count++;
					else {
						compressed.emplace_back(pBlockID, count);
						pBlockID = world[chunk].chunkData[x][y][z];
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
					glm::vec3 pos = to3D(counter);
					uint8_t x = pos.x;
					uint8_t y = pos.z;
					uint8_t z = pos.y;
					world[chunk].chunkData[x][y][z] = blockID;
					counter++;
				}
			}

			enableBit(world[chunk].flags, canBeUpdatedFlag);
		}

		void ResetChunk(const ChunkID& chunk, const glm::vec3& newChunkPos) {
			//Timer timer("ResetChunk");
			world[chunk].chunkPos = newChunkPos;
			UpdateNeighbours(chunk);
			GenerateChunkTerrain(chunk);
			UpdateMesh(chunk);
		}

		void UpdateNeighbours(const ChunkID& chunk) {
			world[chunk].neighborXpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 1,0,0 });
			world[chunk].neighborYpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,1,0 });
			world[chunk].neighborZpos = getChunkID(world[chunk].chunkPos + glm::vec3{ 0,0,1 });

			world[chunk].neighborXneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 1,0,0 });
			world[chunk].neighborYneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,1,0 });
			world[chunk].neighborZneg = getChunkID(world[chunk].chunkPos - glm::vec3{ 0,0,1 });

			if (world[chunk].neighborXpos >= 0) { world[world[chunk].neighborXpos].neighborXneg = chunk; }
			if (world[chunk].neighborXneg >= 0) { world[world[chunk].neighborXneg].neighborXpos = chunk; }

			if (world[chunk].neighborYpos >= 0) { world[world[chunk].neighborYpos].neighborYneg = chunk; }
			if (world[chunk].neighborYneg >= 0) { world[world[chunk].neighborYneg].neighborYpos = chunk; }

			if (world[chunk].neighborZpos >= 0) { world[world[chunk].neighborZpos].neighborZneg = chunk; }
			if (world[chunk].neighborZneg >= 0) { world[world[chunk].neighborZneg].neighborZpos = chunk; }
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {

				memset(&world[chunk].chunkData, 0, sizeof(world[chunk].chunkData));
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap =
						worldNoise.getNoiseFor(
							x + world[chunk].chunkPos.x * chunkSize, 
							z + world[chunk].chunkPos.z * chunkSize);
						//int biomeMap = biomeNoise.getNoiseFor(
						//	x + world[chunk].chunkPos.x * chunkSize,
						//	z + world[chunk].chunkPos.z * chunkSize);
						for (uint8_t y = 0; y < chunkSize; y++) {
						glm::vec3 pos = world[chunk].chunkPos * glm::vec3(chunkSize) + glm::vec3(x, y, z);
						//float noise3D = worldNoise.get3DNoiseFor(pos.x, pos.y, pos.z);
						//if (noise3D < 7) { setBlock(glm::vec3(x, y, z), 1, chunk); }
							if (pos.y == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
							if (pos.y < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
							if (pos.y < heightMap - 3) { setBlock(glm::vec3(x, y, z), 3, chunk); } // @TODO randomnes
							if (pos.y == heightMap && pos.y <= water_level) { setBlock(glm::vec3(x, y, z), 4, chunk); } // @TODO randomnes
							if (pos.y > heightMap && pos.y < water_level) { setBlock(glm::vec3(x, y, z), 5, chunk); }
							//if (pos.y == heightMap && rand() % 100 > 98 && pos.y > water_level) setBlock(pos, 7); // @TODO randomnes
							//if (pos == heightMap && pos > snow_level) { setBlock(glm::vec3(x, y + 1, z), 8, chunk); }
							//if (pos == heightMap && heightMap > water_level && biomeMap > 48) { setBlock(glm::vec3(x, y, z), 4, chunk); }
						}
					}
		}

		void setBlock(const glm::vec3& pos, const BlockID& block, const ChunkID& chunk) {
			if (chunk >= world.size() || chunk < 0) return;
			    int8_t x = static_cast<int8_t>(pos.x);
			    int8_t y = static_cast<int8_t>(pos.y);
			    int8_t z = static_cast<int8_t>(pos.z);
				glm::vec3 blockPos = getBlockPos(pos);
				if (x >= chunkSize) x = blockPos.x;
				if (y >= chunkSize) y = blockPos.y;
				if (z >= chunkSize) z = blockPos.z;
			if (world[chunk].chunkData[x][y][z] == block) return;
				world[chunk].chunkData[x][y][z] = block;
				enableBit(world[chunk].flags, canBeUpdatedFlag);

				if (x == 0) { if (world[chunk].neighborXneg >= 0) { enableBit(world[world[chunk].neighborXneg].flags, canBeUpdatedFlag); } }
				if (y == 0) { if (world[chunk].neighborYneg >= 0) { enableBit(world[world[chunk].neighborYneg].flags, canBeUpdatedFlag); } }
				if (z == 0) { if (world[chunk].neighborZneg >= 0) { enableBit(world[world[chunk].neighborZneg].flags, canBeUpdatedFlag); } }

				if (x == chunkSize - 1) { if (world[chunk].neighborXpos >= 0) { enableBit(world[world[chunk].neighborXpos].flags, canBeUpdatedFlag); } }
				if (y == chunkSize - 1) { if (world[chunk].neighborYpos >= 0) { enableBit(world[world[chunk].neighborYpos].flags, canBeUpdatedFlag); } }
				if (z == chunkSize - 1) { if (world[chunk].neighborZpos >= 0) { enableBit(world[world[chunk].neighborZpos].flags, canBeUpdatedFlag); } }
		}

		void setBlock(const glm::vec3& pos, const BlockID& block) {
			ChunkID chunk = getChunkID(getChunkPos(pos));

			if (chunk >= world.size() || chunk < 0) return;
				glm::vec3  blockPos = getBlockPos(pos);
				int8_t x = static_cast<int8_t>(blockPos.x);
				int8_t y = static_cast<int8_t>(blockPos.y);
				int8_t z = static_cast<int8_t>(blockPos.z);

			if (world[chunk].chunkData[x][y][z] == block) return;

				world[chunk].chunkData[x][y][z] = block;
				enableBit(world[chunk].flags, canBeUpdatedFlag);

				if (x == 0) { if (world[chunk].neighborXneg >= 0) { enableBit(world[world[chunk].neighborXneg].flags, canBeUpdatedFlag); }}
				if (y == 0) { if (world[chunk].neighborYneg >= 0) { enableBit(world[world[chunk].neighborYneg].flags, canBeUpdatedFlag); }}
				if (z == 0) { if (world[chunk].neighborZneg >= 0) { enableBit(world[world[chunk].neighborZneg].flags, canBeUpdatedFlag); }}

				if (x == chunkSize - 1) { if (world[chunk].neighborXpos >= 0) { enableBit(world[world[chunk].neighborXpos].flags, canBeUpdatedFlag); } }
				if (y == chunkSize - 1) { if (world[chunk].neighborYpos >= 0) { enableBit(world[world[chunk].neighborYpos].flags, canBeUpdatedFlag); } }
				if (z == chunkSize - 1) { if (world[chunk].neighborZpos >= 0) { enableBit(world[world[chunk].neighborZpos].flags, canBeUpdatedFlag); } }			
		}

			
		void collide(const glm::vec3& vel, float dt)
		{
			for (int x = p.Position.x - p.Size.x; x < p.Position.x + p.Size.x; x++)
				for (int y = p.Position.y - p.Size.y; y < p.Position.y + 0.7; y++)
					for (int z = p.Position.z - p.Size.z; z < p.Position.z + p.Size.z; z++) {
						auto blockID = getBlock({ x, y, z });
						auto block = blockData[blockID];
				if (block.id != 0 && bitEnabled(block.flags, 0)) {
					if (vel.y > 0) {
						p.Position.y = y - p.Size.y;
						p.velocity.y = 0;
					}
					else if (vel.y < 0) {
						p.m_isOnGround = true;
						p.Position.y = y + p.Size.y + 1;
						p.velocity.y = 0;
					}

					if (vel.x > 0) {
						p.Position.x = x - p.Size.x;
					}
					else if (vel.x < 0) {
						p.Position.x = x + p.Size.x + 1;
					}

					if (vel.z > 0) {
						p.Position.z = z - p.Size.z;
					}
					else if (vel.z < 0) {
						p.Position.z = z + p.Size.z + 1;
					}
				}
			}
		}
		

		void UpdateMesh(const ChunkID& chunk) {
			if (chunk >= world.size() || chunk < 0) return;

				uint32_t offset = 0;
				world[chunk].IndexCount = 0;

				gl::Vertex worldMesh[MaxVertexCount];

				bool show = false;

				auto addFace = [&](const Face& face, const glm::vec3& pos, const uint32_t& texture, const int8_t& type) {
					if (world[chunk].IndexCount > MaxFaceCount * 6) { WC_ERROR("Memory overflow!"); return; }
					
					uint8_t textureSizeX = 1;
					uint8_t textureSizeY = 1;
					
					glm::vec2 TexCoords[4] = {
						glm::vec2(0.0f, 0.0f),
						glm::vec2(0.0f,         textureSizeY),
						glm::vec2(textureSizeX, textureSizeY),
						glm::vec2(textureSizeX, 0.0f),
					};
					for (uint8_t i = 0; i < 4; i++) {
						glm::vec3 Pos = face[i] + pos;
						worldMesh[i + offset] = gl::Vertex(Pos, { TexCoords[i], texture }, type);
						//if (viewFrustum.isBoxInFrustum(world[chunk].chunkPos * glm::vec3(chunkSize) + Pos)) show = true;
					}
					world[chunk].IndexCount += 6;
					offset += 4;
				};
				uint8_t y = 0, x = 0, z = 0;
						for (; y < chunkSize; y++)
						for (x = 0; x < chunkSize; x++)
						for (z = 0; z < chunkSize; z++)
						{
							BlockID block = world[chunk].chunkData[x][y][z];
							BlockID checkBlock;
							if (makeFace({ x,y,z }, chunk, ConnectionType::CONNECT_DEFAULT)) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);									
								}
								else if (world[chunk].neighborYpos >= 0) {
									checkBlock = world[world[chunk].neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);									
								}								

								if (z + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], 0);									
								}
								else if (world[chunk].neighborZpos >= 0) {
									checkBlock = world[world[chunk].neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], 0);									
								}

								if (x + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], 0);									
								}
								else if (world[chunk].neighborXpos >= 0) {
									checkBlock = world[world[chunk].neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], 0);									
								}

								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = world[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], 0);									
								}
								else if (world[chunk].neighborYneg >= 0) {
									checkBlock = world[world[chunk].neighborYneg].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], 0);									
								}

								if (z - 1 >= 0) {
									checkBlock = world[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], 0);
								}
								else if (world[chunk].neighborZneg >= 0) {
									checkBlock = world[world[chunk].neighborZneg].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], 0);									
								}

								if (x - 1 >= 0) {
									checkBlock = world[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], 0);
								}
								else if (world[chunk].neighborXneg >= 0) {
									checkBlock = world[world[chunk].neighborXneg].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 || blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], 0);									
								}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::NO_CONNECT)) // Can make block face
							{
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);
								}
								else if (world[chunk].neighborYpos >= 0) {
									checkBlock = world[world[chunk].neighborYpos].chunkData[x][0][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);									
								}

								if (z + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x][y][z + 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], 0);
								}
								else if (world[chunk].neighborZpos >= 0) {
									checkBlock = world[world[chunk].neighborZpos].chunkData[x][y][0];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(FRONT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::FRONT], 0);									
								}

								if (x + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x + 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], 0);
								}
								else if (world[chunk].neighborXpos >= 0) {
									BlockID checkBlock = world[world[chunk].neighborXpos].chunkData[0][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::RIGHT], 0);									
								}
								//Negative	   																										 					  															    						 
								if (y - 1 >= 0) {
									checkBlock = world[chunk].chunkData[x][y - 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], 0);
								}
								else  if (world[chunk].neighborYneg >= 0) {
									checkBlock = world[world[chunk].neighborYneg].chunkData[x][chunkSize - 1][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(BOTTOM_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BOTTOM], 0);									
								}

								if (z - 1 >= 0) {
									checkBlock = world[chunk].chunkData[x][y][z - 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], 0);
								}
								else if (world[chunk].neighborZneg >= 0) {
									checkBlock = world[world[chunk].neighborZneg].chunkData[x][y][chunkSize - 1];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(BACK_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::BACK], 0);									
								}

								if (x - 1 >= 0) {
									checkBlock = world[chunk].chunkData[x - 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], 0);
								}
								else if (world[chunk].neighborXneg >= 0) {
									checkBlock = world[world[chunk].neighborXneg].chunkData[chunkSize - 1][y][z];
									if (checkBlock == 0 && blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
										addFace(LEFT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::LEFT], 0);									
								}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::FLUID_CONNECT)) // Can make a fluid face
							{
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = world[chunk].chunkData[x][y + 1][z];
									if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType) 
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 1);									
								}
								else if (world[chunk].neighborYpos >= 0) {
									checkBlock = world[world[chunk].neighborYpos].chunkData[x][0][z];
									if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)	
										addFace(TOP_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 1);									
								}

								//if (x + 1 < chunkSize) {
								//	BlockID checkBlock = world[chunk].chunkData[x + 1][y][z];
								//	if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
								//		addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].fIndexCount, foffset, worldFluidMesh);
								//}
								//else if (neighborXpos >= 0) {
								//	BlockID checkBlock = world[neighborYpos].chunkData[0][y][z];
								//	if (blockData[block].blockConnectionType != blockData[checkBlock].blockConnectionType)
								//		addFace(RIGHT_FACE, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(1.0f, 0.0f, 0.0f), world[chunk].fIndexCount, foffset, worldFluidMesh);
								//}
							}

							if (makeFace({ x,y,z }, chunk, ConnectionType::X_CONNECT)) // Can make a fluid face
							{
									addFace(X_FACE1, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);
									addFace(X_FACE2, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);
									//addFace(X_FACE3, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
									//addFace(X_FACE4, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], glm::vec3(0.0f, 1.0f, 0.0f), world[chunk].IndexCount, offset, worldMesh);
							}
						}

				worldIndexBuffer.Bind();

				//if (show && world[chunk].IndexCount > 0) enableBit(world[chunk].flags, showFlag);

				world[chunk].chunkMeshArray.Bind();
				if (world[chunk].IndexCount > 0) world[chunk].chunkMeshBuffer.Update(0, sizeof(worldMesh), &worldMesh);			
		}	

		bool makeFace(const glm::vec3& pos, const ChunkID& chunkID, ConnectionType type) {
			if (pos.x >= chunkSize || pos.y >= chunkSize || pos.z >= chunkSize) return false;
			if (pos.x < 0 || pos.y < 0 || pos.z < 0) return false;
			if (chunkID >= world.size()) return false;
			if (chunkID < 0) return false;
			int8_t x = pos.x;
			int8_t y = pos.y;
			int8_t z = pos.z;
			BlockID block = world[chunkID].chunkData[x][y][z];
			if (block > 0 && blockData[block].blockConnectionType == type) return true;
			return false;
		}				

		BlockID getBlock(const glm::vec3& pos) {
			ChunkID chunk = getChunkID(getChunkPos(pos));
			glm::vec3  blockPos = getBlockPos(pos);
			int8_t x = blockPos.x;
			int8_t y = blockPos.y;
			int8_t z = blockPos.z;
			return world[chunk].chunkData[x][y][z];
		}		

		int16_t getChunkID(const glm::vec3& pos) {
			for (ChunkID i = 0; i < world.size(); i++) {
				if (world[i].chunkPos.x == pos.x &&
					world[i].chunkPos.y == pos.y &&
					world[i].chunkPos.z == pos.z) {
					return i;
				}
			}
			return -1;
		}

		bool cpmparaQuads(glm::vec4 quad1, glm::vec4 quad2) {
			if (quad1.y != quad2.y) return quad1.y < quad2.y;
			if (quad1.x != quad2.x) return quad1.x < quad2.x;
			if (quad1.z != quad2.z) return quad1.z < quad2.z;
			return quad1.w < quad2.w;
		}
	};	
}
#endif