#ifndef MULTIPLAYER_HPP
#define MULTIPLAYER_HPP

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
#include <net/wc_net.hpp>
#include <GUI/Renderer2D.hpp>

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
	static const uint8_t RenderDistance = chunkSize;

	static void AddBlock(const char* script) {
		Block block;

		std::string conType;
		sol::state blockState;
		blockState.script_file(script);
		if (blockState["id"].valid()) block.id = blockState["id"];
		if (blockState["isCollidable"].valid()) block.isCollidable = blockState["isCollidable"];
		if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];

		if (conType == "CONNECT_DEFAULT") block.blockConnectionType = ConnectionType::CONNECT_DEFAULT;
		if (conType == "FLUID_CONNECT")   block.blockConnectionType = ConnectionType::FLUID_CONNECT;
		if (conType == "NO_CONNECT")      block.blockConnectionType = ConnectionType::NO_CONNECT;
		if (conType == "X_CONNECT")      block.blockConnectionType = ConnectionType::X_CONNECT;


		if (blockState["allTextures"].valid()) {
			block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(blockState["allTextures"]);
			block.texture[(int)BlockTexture::BOTTOM] = block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::FRONT] = block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::BACK] = block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::LEFT] = block.texture[(int)BlockTexture::TOP];
			block.texture[(int)BlockTexture::RIGHT] = block.texture[(int)BlockTexture::TOP];
		}
		if (blockState["top"].valid())    block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(blockState["top"]);
		if (blockState["bottom"].valid()) block.texture[(int)BlockTexture::BOTTOM] = assets.LoadTexture(blockState["bottom"]);
		if (blockState["front"].valid())  block.texture[(int)BlockTexture::FRONT] = assets.LoadTexture(blockState["front"]);
		if (blockState["back"].valid())   block.texture[(int)BlockTexture::BACK] = assets.LoadTexture(blockState["back"]);
		if (blockState["left"].valid())   block.texture[(int)BlockTexture::LEFT] = assets.LoadTexture(blockState["left"]);
		if (blockState["right"].valid())  block.texture[(int)BlockTexture::RIGHT] = assets.LoadTexture(blockState["right"]);

		if (blockState["emitLight"].valid()) block.emitLight = blockState["emitLight"];

		blockData[block.id] = block;
	}


	class Multiplayer : public net::client_interface<GameMsg> {
	public:
		Player p;
		uint32_t localPlayerID = 0;
		std::unordered_map<uint32_t, PlayerDescription> players;
		Font font;
		Model model;
		gl::Shader modelShader;
		Multiplayer() {}

		void Create() {
			worldShader.Create("shaderpacks/default/chunkShader.glsl");
			sol::state luaState;
			luaState.script_file("scripts/worldGen.lua");

			worldNoise.lacunarity = luaState["lacunarity"];
			worldNoise.multiplier = luaState["multiplier"];
			worldNoise.octaves = luaState["octaves"];
			worldNoise.persistance = luaState["persistance"];
			worldNoise.scale = luaState["scale"];
			worldNoise.seed = luaState["seed"];

			water_level = luaState["water_level"];

			assets.Create(30, 32, 32);

			//Loading blocks

			luaState.set_function("AddBlock", &AddBlock);
			luaState.open_libraries(sol::lib::base);

			luaState.script_file("scripts/blocks.lua");

			p.InitPlayer({ RenderDistance * RenderDistance / 2 + RenderDistance, RenderDistance * 4 ,RenderDistance * RenderDistance / 2 });
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

			uint32_t indices[MaxFaceCount * 6];
			uint32_t ioffset = 0;
			uint32_t iIndices = 0;
			for (; iIndices < sizeof(indices) / sizeof(uint32_t); iIndices += 6) {
				indices[iIndices + 0] = 0 + ioffset;
				indices[iIndices + 1] = 1 + ioffset;
				indices[iIndices + 2] = 2 + ioffset;

				indices[iIndices + 3] = 2 + ioffset;
				indices[iIndices + 4] = 3 + ioffset;
				indices[iIndices + 5] = 0 + ioffset;

				ioffset += 4;
			}
			modelShader.Create("shaderpacks/default/modelShader.glsl");
			model.Create("assets/models/dancing_vampire.dae");

			//std::string ipAddres;
			//std::cin >> ipAddres;

			Connect("25.104.236.246", 60000); // 25.104.236.246

			worldIndexBuffer.Create(indices, sizeof(indices));
			for (ChunkID chunk = 0; chunk < world.size(); chunk++) UpdateNeighbours(chunk);
			//players[0].nUniqueID = 0;
			//players[0].Position = { RenderDistance * RenderDistance / 2 + RenderDistance, RenderDistance * 4 ,RenderDistance * RenderDistance / 2 };
		}

		bool bWaitingForConnection = true;

		PlayerDescription descPlayer;

		void Update(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {
			p.UpdatePlayer(windpos, windsize, CenterMouse);

			assets.Bind();
			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.camera.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("u_Projection", p.projection);

			// camera/view transformation
			worldShader.setMat4("u_View", p.GetView());

			viewFrustum.update(p.projection * p.GetView());
			uint8_t chunkHalf = RenderDistance / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position);

			if (IsConnected())
			{
				while (!Incoming().empty())
				{
					auto msg = Incoming().pop_front().msg;

					switch (msg.header.id)
					{
					case(GameMsg::Client_Accepted):
					{
						WC_INFO("Server accepted client - you're in!");
						net::message<GameMsg> msg;
						msg.header.id = GameMsg::Client_RegisterWithServer;
						descPlayer.Position = glm::vec3(0.f);
						msg << descPlayer;
						Send(msg);
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
						glm::vec4 blockData = glm::vec4(0.f);
						msg >> blockData;
						setBlock(blockData, blockData.w);
						break;
					}
					}
				}
			}

			if (bWaitingForConnection) Renderer2D::DrawTexts("Waiting for connection", font, windsize / glm::vec2(2));

			for (ChunkID i = 0; i < world.size(); i++) {

				if (world[i].IndexCount > 0 && ShowChunk(i)) {
					glm::vec3 pos = world[i].chunkPos * glm::vec3(chunkSize);
					world[i].chunkMeshArray.Bind();
					worldShader.setMat4("u_Model", glm::translate(glm::mat4(1.0f), pos)); // calculate the model matrix for each object and pass it to shader before drawing
					Renderer::DrawIndexed(world[i].IndexCount);
				}

				glm::vec3 currChunkPos = world[i].chunkPos;
				if (currChunkPos.x < currentPlayerPos.x - chunkHalf) ResetChunk(i, glm::vec3(currentPlayerPos.x + chunkHalf - 1, currChunkPos.y, currChunkPos.z));
				if (currChunkPos.x > currentPlayerPos.x + chunkHalf) ResetChunk(i, glm::vec3(currentPlayerPos.x - chunkHalf + 1, currChunkPos.y, currChunkPos.z));

				if (currChunkPos.z < currentPlayerPos.z - chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z + chunkHalf - 1));
				if (currChunkPos.z > currentPlayerPos.z + chunkHalf) ResetChunk(i, glm::vec3(currChunkPos.x, currChunkPos.y, currentPlayerPos.z - chunkHalf + 1));

				// Updating the chunk`s mesh
				if (!world[i].generated) { GenerateChunkTerrain(i);	world[i].generated = true; }
				if (world[i].canBeUpdated) { UpdateMesh(i);	world[i].canBeUpdated = false; }
			}

			net::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_UpdatePlayer;
			players[localPlayerID].Position = p.Position;
			msg << players[localPlayerID];
			Send(msg);

			modelShader.use();
			modelShader.setMat4("projection", p.projection);
			modelShader.setMat4("view", p.GetView());

			for (auto& player : players) {
				if (player.second.nUniqueID != localPlayerID) {

					glm::mat4 Model = glm::mat4(1.0f);
					// Draw Players
					// render the loaded model
					Model = glm::translate(Model, player.second.Position); // translate it down so it's at the center of the scene
					Model = glm::scale(Model, glm::vec3(0.01f));	// it's a bit too big for our scene, so scale it down
					modelShader.setMat4("model", Model);
					glDisable(GL_CULL_FACE);
					glDisable(GL_BLEND);
					model.Draw(modelShader);
					glEnable(GL_CULL_FACE);
					glEnable(GL_BLEND);
				}
			}

			// Render2D Stuff
			Renderer2D::DrawTexts("X: " + std::to_string(p.Position.x) + " Y: " + std::to_string(p.Position.y) + " Z: " + std::to_string(p.Position.z), font, { 25.0f, 60 });
			Renderer2D::DrawTexts("Pitch: " + std::to_string(p.camera.Pitch) + " Yaw: " + std::to_string(p.camera.Yaw), font, { 25.0f, 100 });
			Renderer2D::DrawTexts(
				"ChunkX: " + std::to_string(currentPlayerPos.x) +
				" ChunkY: " + std::to_string(currentPlayerPos.y) +
				" ChunkZ: " + std::to_string(currentPlayerPos.z), font, { 25.0f, 140 });
			Renderer2D::DrawTexts("Players ingame: " + std::to_string(players.size()), font, { 25.0f, 180 });
			Renderer2D::DrawTexts("Local player id: " + std::to_string(localPlayerID), font, { 25.0f, 220 });
		}

		void OnInput(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
			bool bBreak = Mouse::isButtonPressed() == Mouse::MouseButton::LBUTTON;
			bool bPlace = Mouse::isButtonPressed() == Mouse::MouseButton::RBUTTON;
			if (bBreak || bPlace) {
				net::message<GameMsg> msg;
				float x = 0.0f;
				float y = 0.0f;
				float z = 0.0f;
				glm::vec3 m_rayLastPos = glm::vec3(0.0f);
				Ray ray(p.Position);
				while (ray.getLength() < 4) {
					ray.Step(p.camera.Yaw, p.camera.Pitch, 0.05f);

					if (getBlock(ray.getEnd()) > 0)
						if (bBreak) {
							setBlock(glm::floor(ray.getEnd()), 0);
							msg.header.id = GameMsg::BlockEdit;
							glm::vec4 test = glm::vec4(ray.getEnd(), 0);
							msg << test;
							Send(msg);
							break;
						}
						else if (bPlace) {
							setBlock(glm::floor(m_rayLastPos), p.ItemHolding);
							msg.header.id = GameMsg::BlockEdit;
							glm::vec4 test = glm::vec4(m_rayLastPos, p.ItemHolding);
							msg << test;
							Send(msg);
							break;
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
					for (uint8_t y = 0; y < chunkSize; y++) {
						glm::vec3 pos = world[chunk].chunkPos * glm::vec3(chunkSize) + glm::vec3(x, y, z);
						if (pos.y == heightMap) { setBlock(glm::vec3(x, y, z), 1, chunk); }
						if (pos.y < heightMap) { setBlock(glm::vec3(x, y, z), 2, chunk); }
						if (pos.y < heightMap - 3) { setBlock(glm::vec3(x, y, z), 3, chunk); } // @TODO randomnes
						if (pos.y == heightMap && pos.y <= water_level) { setBlock(glm::vec3(x, y, z), 4, chunk); } // @TODO randomnes
						if (pos.y > heightMap && pos.y < water_level) { setBlock(glm::vec3(x, y, z), 5, chunk); }
					}
				}
		}

		void setBlock(const glm::vec3& pos, const BlockID& block, const ChunkID& chunk) {
			if (chunk >= world.size() || chunk < 0) return;
			int8_t x = glm::floor(pos.x);
			int8_t y = glm::floor(pos.y);
			int8_t z = glm::floor(pos.z);
			glm::vec3 blockPos = getBlockPos(pos);
			if (x >= chunkSize) x = blockPos.x;
			if (y >= chunkSize) y = blockPos.y;
			if (z >= chunkSize) z = blockPos.z;
			if (world[chunk].chunkData[x][y][z] == block) return;
			world[chunk].chunkData[x][y][z] = block;
			world[chunk].canBeUpdated = true;

			if (x == 0) { if (world[chunk].neighborXneg >= 0) { world[world[chunk].neighborXneg].canBeUpdated = true; } }
			if (y == 0) { if (world[chunk].neighborYneg >= 0) { world[world[chunk].neighborYneg].canBeUpdated = true; } }
			if (z == 0) { if (world[chunk].neighborZneg >= 0) { world[world[chunk].neighborZneg].canBeUpdated = true; } }

			if (x == chunkSize - 1) { if (world[chunk].neighborXpos >= 0) { world[world[chunk].neighborXpos].canBeUpdated = true; } }
			if (y == chunkSize - 1) { if (world[chunk].neighborYpos >= 0) { world[world[chunk].neighborYpos].canBeUpdated = true; } }
			if (z == chunkSize - 1) { if (world[chunk].neighborZpos >= 0) { world[world[chunk].neighborZpos].canBeUpdated = true; } }
		}

		void setBlock(const glm::vec3& pos, const BlockID& block) {
			int16_t chunk = getChunkID(getChunkPos(pos));

			if (chunk >= world.size() || chunk < 0) return;
			glm::vec3  blockPos = getBlockPos(pos);
			int8_t x = blockPos.x;
			int8_t y = blockPos.y;
			int8_t z = blockPos.z;

			if (world[chunk].chunkData[x][y][z] == block) return;

			world[chunk].chunkData[x][y][z] = block;
			world[chunk].canBeUpdated = true;

			if (x == 0) { if (world[chunk].neighborXneg >= 0) { world[world[chunk].neighborXneg].canBeUpdated = true; } }
			if (y == 0) { if (world[chunk].neighborYneg >= 0) { world[world[chunk].neighborYneg].canBeUpdated = true; } }
			if (z == 0) { if (world[chunk].neighborZneg >= 0) { world[world[chunk].neighborZneg].canBeUpdated = true; } }

			if (x == chunkSize - 1) { if (world[chunk].neighborXpos >= 0) { world[world[chunk].neighborXpos].canBeUpdated = true; } }
			if (y == chunkSize - 1) { if (world[chunk].neighborYpos >= 0) { world[world[chunk].neighborYpos].canBeUpdated = true; } }
			if (z == chunkSize - 1) { if (world[chunk].neighborZpos >= 0) { world[world[chunk].neighborZpos].canBeUpdated = true; } }
		}

		void UpdateMesh(const ChunkID& chunk) {
			if (chunk >= world.size() || chunk < 0) return;

			uint32_t offset = 0;
			world[chunk].IndexCount = 0;

			gl::Vertex worldMesh[MaxVertexCount];

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
				for (uint8_t i = 0; i < 4; i++) { worldMesh[i + offset] = gl::Vertex(face[i] + pos, { TexCoords[i], texture }, type); }
				world[chunk].IndexCount += 6;
				offset += 4;
			};

			for (uint8_t y = 0; y < chunkSize; y++)
				for (uint8_t x = 0; x < chunkSize; x++)
					for (uint8_t z = 0; z < chunkSize; z++)
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
						}

						if (makeFace({ x,y,z }, chunk, ConnectionType::X_CONNECT)) // Can make a fluid face
						{
							addFace(X_FACE1, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);
							addFace(X_FACE2, glm::vec3(x, y, z), blockData[block].texture[(int)BlockTexture::TOP], 0);
						}
					}
			worldIndexBuffer.Bind();

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

		gl::Shader worldShader;

		gl::IndexBuffer worldIndexBuffer;
		std::array<Chunk, RenderDistance* RenderDistance* RenderDistance> world;
		Noise worldNoise;
		Frustum viewFrustum;

		int8_t water_level = 0;
	};
}
#endif