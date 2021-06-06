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
#include "../Game Mechanics/ParticleSystem.hpp"

//const uint32_t numThreads = std::thread::hardware_concurrency();

uint32_t heapMemory = 0;

#define MODEL1

namespace wc {

	static AssetManager assets;
	static const uint8_t RenderDistance = 16;	
	Block blockData[256];
	// @TODO: Sol error handling

	class Singleplayer {
	private:
		// Player related
		Player p;
		glm::mat4 projection = glm::mat4(0.f);
		Camera camera;
		float MouseSensitivity = 5;
		Random numberGen;
		LineBatcher lineBatcher;

		gl::Shader chunkShader;

		gl::IndexBuffer worldIndexBuffer;
		std::array<Chunk, RenderDistance * RenderDistance * RenderDistance> chunks;
		Noise worldNoise;
		Noise temperatureNoise;
		Frustum viewFrustum;

		//ParticleSystem particleSystem;
		//ParticleProps m_Particle;

		int8_t water_level = 0;

		Biome mountainBiome;

		gl::Texture cursorTex;
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

			if (conType == "CONNECT_DEFAULT") block.blockConnectionType = ConnectionType::CONNECT_DEFAULT;
			else if (conType == "FLUID_CONNECT")   block.blockConnectionType = ConnectionType::FLUID_CONNECT;
			else if (conType == "NO_CONNECT")      block.blockConnectionType = ConnectionType::NO_CONNECT;
			else if (conType == "X_CONNECT")      block.blockConnectionType = ConnectionType::X_CONNECT;

			if (blockState["allTextures"].valid()) {
				block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(blockState["allTextures"]);
				block.texture[(int)BlockTexture::BOTTOM] = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::FRONT] = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::BACK] = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::LEFT] = block.texture[(int)BlockTexture::TOP];
				block.texture[(int)BlockTexture::RIGHT] = block.texture[(int)BlockTexture::TOP];
			}
			else {
				if (blockState["top"].valid())    block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(blockState["top"]);
				if (blockState["bottom"].valid()) block.texture[(int)BlockTexture::BOTTOM] = assets.LoadTexture(blockState["bottom"]);
				if (blockState["front"].valid())  block.texture[(int)BlockTexture::FRONT] = assets.LoadTexture(blockState["front"]);
				if (blockState["back"].valid())   block.texture[(int)BlockTexture::BACK] = assets.LoadTexture(blockState["back"]);
				if (blockState["left"].valid())   block.texture[(int)BlockTexture::LEFT] = assets.LoadTexture(blockState["left"]);
				if (blockState["right"].valid())  block.texture[(int)BlockTexture::RIGHT] = assets.LoadTexture(blockState["right"]);
			}

			//if (blockState["emitLight"].valid()) if (blockState["emitLight"]) enableBit(block.flags, emitLightFlag);

			blockData[block.id] = block;
		}

		static void AddBlock(const Block& block) {
			blockData[block.id] = block;
		}

	public:
		Font font;

		void Create() {
			chunkShader.Create("shaderpacks/default/chunkShader.glsl");

			sol::state worldGenState;
			worldGenState.new_usertype<glm::vec2>("vec2", sol::constructors<void(), void(float, float), void(float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y);
			worldGenState.new_usertype<glm::vec3>("vec3", sol::constructors<void(), void(float, float, float), void(float)>(), "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z, "r", &glm::vec3::r, "g", &glm::vec3::g, "b", &glm::vec3::b);
			worldGenState.new_usertype<glm::vec4>("vec4", sol::constructors<void(), void(float, float, float, float), void(float)>(), "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w, "r", &glm::vec4::r, "g", &glm::vec4::g, "b", &glm::vec4::b, "a", &glm::vec4::a);

			worldGenState.new_usertype<glm::ivec2>("ivec2", sol::constructors<void(), void(int, int), void(int)>(), "x", &glm::ivec2::x, "y", &glm::ivec2::y);
			worldGenState.new_usertype<glm::ivec3>("ivec3", sol::constructors<void(), void(int, int, int), void(int)>(), "x", &glm::ivec3::x, "y", &glm::ivec3::y, "z", &glm::ivec3::z, "r", &glm::ivec3::r, "g", &glm::ivec3::g, "b", &glm::ivec3::b);
			worldGenState.new_usertype<glm::ivec4>("ivec4", sol::constructors<void(), void(int, int, int, int), void(int)>(), "x", &glm::ivec4::x, "y", &glm::ivec4::y, "z", &glm::ivec4::z, "w", &glm::ivec4::w, "r", &glm::ivec4::r, "g", &glm::ivec4::g, "b", &glm::ivec4::b, "a", &glm::ivec4::a);

			worldGenState.new_usertype<Noise>("Noise", sol::constructors<void()>(), "lacunarity", &Noise::lacunarity, "multiplier", &Noise::multiplier, "octaves", &Noise::octaves, "persistance", &Noise::persistance, "scale", &Noise::scale, "seed", &Noise::seed);
			worldGenState.new_usertype<Block>("Block", sol::constructors<void()>(), "id", &Block::id, "isCollidable", &Block::isCollidable, "texture", &Block::texture, "blockConnectionType", &Block::blockConnectionType);

			worldGenState.script_file("scripts/worldGen.lua");

			if (worldGenState["noise"].valid()) worldNoise = worldGenState["noise"];
			worldNoise.scale = 1.f / worldNoise.scale;

			if (worldGenState["TempNoise"].valid()) temperatureNoise = worldGenState["TempNoise"];
			temperatureNoise.scale = 1.f / temperatureNoise.scale;

			if (worldGenState["water_level"].valid()) water_level = worldGenState["water_level"];
			//snow_level = noiseState["snow_level"];

			//biomeNoise.lacunarity = 2;
			//biomeNoise.multiplier = 64;
			//biomeNoise.octaves = 2;
			//biomeNoise.persistance = 0.5;
			//biomeNoise.scale = 90;
			//biomeNoise.seed = 10;

			assets.Create(30, 32, 32);

			//Loading blocks

			worldGenState.new_usertype<Block>("Block", sol::constructors<void()>(), "id", &Block::id, "isCollidable", &Block::isCollidable);
			worldGenState.set_function("AddBlockScript", &Singleplayer::AddBlockScript);
			worldGenState.set_function("AddBlock", &Singleplayer::AddBlock);
			worldGenState.open_libraries(sol::lib::base);

			worldGenState.script_file("scripts/blocks.lua");

			p.Position = { (RenderDistance * RenderDistance * 0.5f + RenderDistance), RenderDistance * 4 , (RenderDistance * RenderDistance * 0.5f) };

			lineBatcher.Create();
			//particleSystem.Create();

			for (ChunkID chunkID = 0; chunkID < chunks.size(); chunkID++) {
				//Configuring the vertex array
				chunks[chunkID].chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
				chunks[chunkID].chunkMeshArray.Create();
				Renderer::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
				Renderer::VertexAttribPointer(1, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
				Renderer::VertexAttribPointer(3, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, type)); // type attribute
				chunks[chunkID].chunkPos = to3D(chunkID, glm::ivec3(RenderDistance));
			}
			
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

			load("assets/textures/misc/cursor2.png", cursorTex);

			mountainBiome.biomeTemperature = 20;
#ifdef MODEL
			modelShader.Create("shaderpacks/default/modelShader.glsl");
			model.Create("assets/models/dancing_vampire.dae");
			animation.Create("assets/models/dancing_vampire.dae", &model);
			animator.PlayAnimation(&animation);
#endif // MODEL
			numberGen.seed = worldNoise.seed;
			//GenTerrain = GenerateChunkTerrain;
			//m_Particle.ColorBegin = { 254 / 255.f, 212 / 255.f, 123 / 255.f, 1.f };
			//m_Particle.ColorEnd = { 254 / 255.f, 109 / 255.f, 41 / 255.f, 1.f };
			//m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.f;
			//m_Particle.LifeTime = 1.f;
			//m_Particle.Velocity = { 0.f, 0.f, 0.f };
			//m_Particle.VelocityVariation = { 3.f, 1.f, 0.f };
			//m_Particle.Position = { 0.f, 0.f, 0.f };
		}

		void Update(const glm::vec2& windsize, const float& deltaTime) {

			p.Position.x += p.velocity.x * deltaTime;
			collide({ p.velocity.x,0.f,0.f });

			p.Position.y += p.velocity.y * deltaTime;
			collide({ 0.f,p.velocity.y,0.f });

			p.Position.z += p.velocity.z * deltaTime;
			collide({ 0.f,0.f,p.velocity.z });

			camera.Position =  p.Position;
			camera.Yaw = p.rotation.x;
			camera.Pitch = p.rotation.y;
			//camera.center(p.Position, p.rotation.x);
			p.velocity = { 0,0,0 };
	
			assets.Bind();
			// activate shader
			chunkShader.use();

			float Far = chunkSize * RenderDistance; // 1100
			projection = glm::perspective(glm::radians(camera.FOV), windsize.x / windsize.y, 0.1f, Far);

			chunkShader.setMat4("u_Projection", projection);

			//chunkShader.setVec3("viewPos", camera.Position);

			// pass the delta time variable to the shader
			//chunkShader.setFloat("deltaTime", deltaTime);

			// camera/view transformation
			chunkShader.setMat4("u_View", camera.GetViewMatrix());

			viewFrustum.update(projection * camera.GetViewMatrix());
			uint8_t chunkHalf = RenderDistance / 2;
			glm::vec3 currentPlayerPos = getChunkPos(p.Position);
			for (ChunkID i = 0; i < chunks.size(); i++)
				if (!chunks[i].generated) { GenerateChunkTerrain(i); chunks[i].generated = true; }

			for (ChunkID i = 0; i < chunks.size(); i++) {

				if (chunks[i].IndexCount > 0 && ShowChunk(i)) {
					glm::vec3 pos = chunks[i].chunkPos * glm::ivec3(chunkSize);
					chunks[i].chunkMeshArray.Bind();
					//chunkShader.setMat4("u_Model", glm::translate(glm::mat4(1.f), pos)); // calculate the model matrix for each object and pass it to shader before drawing
					chunkShader.setVec3("chunkPos", chunks[i].chunkPos);
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
			modelShader.setMat4("projection", projection);
			modelShader.setMat4("view", camera.GetViewMatrix());

			auto transforms = animator.GetPoseTransforms();

			for (uint32_t i = 0; i < MAX_BONE_WEIGHTS; i++) { //transforms.size()
				std::string Transform = "finalBonesMatrices[" + std::to_string(i) + "]";
				modelShader.setMat4(Transform.c_str(), transforms[i]);
			}
			// render the loaded model
			animator.UpdateAnimation(deltaTime);
			glm::mat4 Model = glm::mat4(1.f);
			Model = glm::translate(Model, p.Position); // translate it down so it's at the center of the scene
			//Model = glm::scale(Model, glm::vec3(1.f));	// it's a bit too big for our scene, so scale it down
			modelShader.setMat4("model", Model);
			glDisable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			model.Draw(modelShader);
			glEnable(GL_CULL_FACE);
			glEnable(GL_BLEND);
#endif
			lineBatcher.UpdateUniforms(camera.GetViewMatrix(), projection);	

			//particleSystem.OnUpdate(deltaTime);
			//particleSystem.Emit(m_Particle);
			Renderer2D::SetLineWidth(2.f);
			lineBatcher.Flush();
			Renderer2D::SetLineWidth(1.f);

			// Render 2D Stuff
			Renderer2D::DrawTexts("X: " + std::to_string(p.Position.x) + " Y: " + std::to_string(p.Position.y) + " Z: " + std::to_string(p.Position.z), font, { 25.f, 60.f });
			Renderer2D::DrawTexts("Pitch: " + std::to_string(p.rotation.x) + " Yaw: " + std::to_string(p.rotation.y), font, { 25.f, 100.f });
			Renderer2D::DrawTexts(
				"ChunkX: " + std::to_string(currentPlayerPos.x) +
				" ChunkY: " + std::to_string(currentPlayerPos.y) +
				" ChunkZ: " + std::to_string(currentPlayerPos.z), font, { 25.f, 140 });
			Renderer2D::DrawTexts("Is on ground: " + std::to_string(p.m_isOnGround), font, { 25.f, 180.f });		
			Renderer2D::DrawTexts("FPS: " + std::to_string((int)(1 / deltaTime)) + " Frametime: " + std::to_string(deltaTime * 1000), font, { 25.f, 20.f });
			Renderer2D::DrawTexts("Heap Memory: " + std::to_string(heapMemory) + " bytes", font, { 25.f, 220.f });
			Renderer2D::DrawTexts("Current block: " + std::to_string(p.ItemHolding), font, { 25.f, 260.f });
			
			glm::vec2 cursorSize = cursorTex.GetSize() * glm::ivec2(1.4f);
			glm::vec2 cursorPos = {windsize.x * 0.5f - cursorSize.x, windsize.y * 0.5f - cursorSize.y };
			Renderer2D::DrawQuad(cursorPos, cursorSize, cursorTex);
		}

		void OnInput(const float& deltaTime, const glm::ivec2& windpos, const glm::ivec2& windSize, const bool& CenterMouse) {			
			float yaw = glm::radians(p.rotation.x);
			float yaw90 = glm::radians(p.rotation.x + 90.f);
			if (Keyboard::isKeyPressed(Keyboard::Key::W)) { // Front
				p.velocity.x += glm::cos(yaw) * p.MovementSpeed;
				p.velocity.z += glm::sin(yaw) * p.MovementSpeed;
			}

			else if (Keyboard::isKeyPressed(Keyboard::Key::S)) { // Back
				p.velocity.x -= glm::cos(yaw) * p.MovementSpeed;
				p.velocity.z -= glm::sin(yaw) * p.MovementSpeed;
			}
			if (Keyboard::isKeyPressed(Keyboard::Key::A)) { // Left
				p.velocity.x -= glm::cos(yaw90) * p.MovementSpeed;
				p.velocity.z -= glm::sin(yaw90) * p.MovementSpeed;
			}
			else if (Keyboard::isKeyPressed(Keyboard::Key::D)) { // Right
				p.velocity.x += glm::cos(yaw90) * p.MovementSpeed;
				p.velocity.z += glm::sin(yaw90) * p.MovementSpeed;
			}

			if (Keyboard::isKeyPressed(Keyboard::Key::Space))
					p.velocity.y += p.MovementSpeed;
			else if (Keyboard::isKeyPressed(Keyboard::Key::LShift)) p.velocity.y -= p.MovementSpeed;	
			
			if (Keyboard::isKeyPressed(Keyboard::Key::C)) { camera.FOV = 10; MouseSensitivity = 18;}
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

			if (mouseScrolled) {
				if (scrollY < 0) p.ItemHolding--;
				else p.ItemHolding++;
			}
			
			int16_t xt, yt;

			glm::ivec2 pos = wc::Mouse::GetMousePos();

			xt = (int16_t)(windpos.x + windSize.x * 0.5f);
			yt = (int16_t)(windpos.y + windSize.y * 0.5f);

			float ms = 1 / MouseSensitivity;

			bool invertMouse = false;
			if (invertMouse) p.rotation.x += (xt - pos.x) * ms;
			else p.rotation.x -= (xt - pos.x) * ms;

			p.rotation.y += (yt - pos.y) * ms;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (p.rotation.y >  89.f)p.rotation.y =  89.f;
			if (p.rotation.y < -89.f)p.rotation.y = -89.f;
			
			if (p.rotation.x > 360.f)p.rotation.x = 0.f;
			if (p.rotation.x < 0.f)  p.rotation.x = 360.f;

			//camera.move(p.Position, p.rotation.y);
			if (CenterMouse) {
				camera.UpdateCameraAngles();
				wc::Mouse::SetMousePosition(xt, yt);
			}				
			
			bool bBreak = wc::mouseButton == GLFW_MOUSE_BUTTON_LEFT && wc::mouseAction == GLFW_PRESS && wc::mouseUsed;
			bool bPlace = wc::mouseButton == GLFW_MOUSE_BUTTON_RIGHT && wc::mouseAction == GLFW_PRESS  && wc::mouseUsed;
			bool bPick = wc::mouseButton == 2 && wc::mouseAction == GLFW_PRESS && wc::mouseUsed;

			// @TODO: FIX
			glm::vec3 m_rayLastPos = glm::vec3(0.f);
			Ray ray(p.Position);
			bool bShow = true;
			while (ray.getLength() < 6.f) {
				ray.m_rayEnd += camera.Front * 0.5f;
					BlockID block = getBlock(ray.getEnd());
					if (block > 0 && block != 5)
					{
						if (bShow) {

							if (bPick)
								p.ItemHolding = getBlock(ray.getEnd());						

							if (bBreak) { 
								setBlock(glm::floor(ray.getEnd()), 0);
								break; 
							}
							else if (bPlace) {
								setBlock(glm::floor(m_rayLastPos), p.ItemHolding); 
								break; 
							}

							glm::vec3 blockiPos = glm::floor(ray.getEnd()) - glm::vec3(0.5f);
							lineBatcher.DrawLine(blockiPos, blockiPos + glm::vec3(0.f, 1.f, 0.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos, blockiPos + glm::vec3(1.f, 0.f, 0.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 0.f, 0.f), blockiPos + glm::vec3(1.f, 1.f, 0.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 1.f, 0.f), blockiPos + glm::vec3(0.f, 1.f, 0.f), glm::vec4(0, 0, 0, 1));

							lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 0.f, 1.f), blockiPos + glm::vec3(0.f, 1.f, 1.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 0.f, 1.f), blockiPos + glm::vec3(1.f, 0.f, 1.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 0.f, 1.f), blockiPos + glm::vec3(1.f, 1.f, 1.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 1.f, 1.f), blockiPos + glm::vec3(0.f, 1.f, 1.f), glm::vec4(0, 0, 0, 1));

							lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 0.f, 1.f), blockiPos, glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 0.f, 1.f), blockiPos + glm::vec3(1.f, 0.f, 0.f), glm::vec4(0, 0, 0, 1));

							lineBatcher.DrawLine(blockiPos + glm::vec3(0.f, 1.f, 1.f), blockiPos + glm::vec3(0.f, 1.f, 0.f), glm::vec4(0, 0, 0, 1));
							lineBatcher.DrawLine(blockiPos + glm::vec3(1.f, 1.f, 1.f), blockiPos + glm::vec3(1.f, 1.f, 0.f), glm::vec4(0, 0, 0, 1));
							bShow = false;
						}
					}					
					m_rayLastPos = ray.getEnd();
				
			}
		}

	private:
		//Chunk managing

		void collide(const glm::vec3& vel) // @TODO: Redo the offset
		{
		for (int x = p.Position.x; x < p.Position.x + p.Size.x; x++)
			for (int y = p.Position.y; y < p.Position.y + p.Size.y; y++)
				for (int z = p.Position.z; z < p.Position.z + p.Size.z; z++)
				{
				wc::BlockID blockID = getBlock({ x, y, z });
				wc::Block block = blockData[blockID];
				if (block.id != 0 && block.isCollidable) {
					if (vel.y > 0) {
						p.Position.y = y - p.Size.y;
						p.velocity.y = 0;
					}
					else if (vel.y < 0) {
						p.m_isOnGround = true;
						p.Position.y = y + p.Size.y;
						p.velocity.y = 0;
					}

					if (vel.x > 0) {
						p.Position.x = x - p.Size.x;
					}
					else if (vel.x < 0) {
						p.Position.x = x + p.Size.x;
					}

					if (vel.z > 0) {
						p.Position.z = z - p.Size.z;
					}
					else if (vel.z < 0) {
						p.Position.z = z + p.Size.z;
					}
				}
			}
		}

		bool ShowChunk(const ChunkID& chunk) { //@TODO: Optimize
			glm::vec3 pos1 = chunks[chunk].chunkPos * glm::ivec3(chunkSize);
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

			//Decompress(data, chunk);			
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
		}

		void ResetChunk(const ChunkID& chunk, const glm::ivec3& newChunkPos) {
			chunks[chunk].chunkPos = newChunkPos;
			UpdateNeighbours(chunk);
			GenerateChunkTerrain(chunk);
			UpdateMesh(chunk);
		}

		void UpdateNeighbours(const ChunkID& chunk) {
			chunks[chunk].neighborXpos = getChunkID(chunks[chunk].chunkPos + glm::ivec3{ 1,0,0 });
			chunks[chunk].neighborYpos = getChunkID(chunks[chunk].chunkPos + glm::ivec3{ 0,1,0 });
			chunks[chunk].neighborZpos = getChunkID(chunks[chunk].chunkPos + glm::ivec3{ 0,0,1 });

			chunks[chunk].neighborXneg = getChunkID(chunks[chunk].chunkPos - glm::ivec3{ 1,0,0 });
			chunks[chunk].neighborYneg = getChunkID(chunks[chunk].chunkPos - glm::ivec3{ 0,1,0 });
			chunks[chunk].neighborZneg = getChunkID(chunks[chunk].chunkPos - glm::ivec3{ 0,0,1 });

			if (chunks[chunk].neighborXpos >= 0) { chunks[chunks[chunk].neighborXpos].neighborXneg = chunk; }
			if (chunks[chunk].neighborXneg >= 0) { chunks[chunks[chunk].neighborXneg].neighborXpos = chunk; }

			if (chunks[chunk].neighborYpos >= 0) { chunks[chunks[chunk].neighborYpos].neighborYneg = chunk; }
			if (chunks[chunk].neighborYneg >= 0) { chunks[chunks[chunk].neighborYneg].neighborYpos = chunk; }

			if (chunks[chunk].neighborZpos >= 0) { chunks[chunks[chunk].neighborZpos].neighborZneg = chunk; }
			if (chunks[chunk].neighborZneg >= 0) { chunks[chunks[chunk].neighborZneg].neighborZpos = chunk; }
		}

		void GenerateChunkTerrain(const ChunkID& chunk) {
				memset(&chunks[chunk].chunkData, 0, sizeof(chunks[chunk].chunkData));
				for (uint8_t z = 0; z < chunkSize; z++)
				for (uint8_t x = 0; x < chunkSize; x++) {
					int heightMap =	(int)worldNoise.getNoiseFor(x + chunks[chunk].chunkPos.x * chunkSize, z + chunks[chunk].chunkPos.z * chunkSize);
					//int TempMap   = (int)temperatureNoise.getNoiseFor(x + chunks[chunk].chunkPos.x * chunkSize,	z + chunks[chunk].chunkPos.z * chunkSize);

					for (uint8_t y = 0; y < chunkSize; y++) {
					glm::ivec3 pos = chunks[chunk].chunkPos * glm::ivec3(chunkSize) + glm::ivec3(x, y, z);
					//float noise3D = chunks.get3DNoiseFor(pos.x, pos.y, pos.z);
					//if (noise3D < 7) { setBlock(glm::vec3(x, y, z), 1, chunk); }


						if (pos.y == heightMap) { 
							if (pos.y <= water_level)
								setBlock(glm::ivec3(x, y, z), 4, chunk); // @TODO randomnes
							else
								setBlock(glm::ivec3(x, y, z), 1, chunk);
							//if (TempMap > mountainBiome.biomeTemperature) setBlock(glm::ivec3(x, y ,z), 4, chunk);
						}
						if (pos.y < heightMap) { setBlock(glm::ivec3(x, y, z), 2, chunk); }
						if (pos.y < heightMap - numberGen.asInt() % 3) { setBlock(glm::ivec3(x, y, z), 3, chunk); } // @TODO randomnes
						if (pos.y > heightMap && pos.y < water_level) { setBlock(glm::ivec3(x, y, z), 5, chunk); }
					}
				}
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block, const ChunkID& chunk) {
			int8_t x = pos.x;
			int8_t y = pos.y;
			int8_t z = pos.z;

			//@TODO: Fix chunk border issue
			if (chunks[chunk].chunkData[x][y][z] == block) return;

			chunks[chunk].chunkData[x][y][z] = block;
			chunks[chunk].canBeUpdated = true;
			chunks[chunk].empty = false;

			if (x == 0) { if (chunks[chunk].neighborXneg >= 0) { chunks[chunks[chunk].neighborXneg].canBeUpdated = true; } }
			if (y == 0) { if (chunks[chunk].neighborYneg >= 0) { chunks[chunks[chunk].neighborYneg].canBeUpdated = true; } }
			if (z == 0) { if (chunks[chunk].neighborZneg >= 0) { chunks[chunks[chunk].neighborZneg].canBeUpdated = true; } }

			if (x == chunkSizeMinusOne) { if (chunks[chunk].neighborXpos >= 0) { chunks[chunks[chunk].neighborXpos].canBeUpdated = true; } }
			if (y == chunkSizeMinusOne) { if (chunks[chunk].neighborYpos >= 0) { chunks[chunks[chunk].neighborYpos].canBeUpdated = true; } }
			if (z == chunkSizeMinusOne) { if (chunks[chunk].neighborZpos >= 0) { chunks[chunks[chunk].neighborZpos].canBeUpdated = true; } }
		}

		void setBlock(const glm::ivec3& pos, const BlockID& block) {
			ChunkID chunk = getChunkID(getChunkPos(pos));

			glm::ivec3 blockPos = getBlockPos(pos);
			setBlock(blockPos, block, chunk);
		}

		void UpdateMesh(const ChunkID& chunk) {
			//wc::Timer timer("UpdateMesh");
			uint32_t offset = 0;
			chunks[chunk].IndexCount = 0;

			gl::Vertex chunkMesh[MaxVertexCount];

			worldIndexBuffer.Bind();
			chunks[chunk].chunkMeshArray.Bind();

			auto addFace = [&](const Face& face, const uint8_t& x, const uint8_t& y, const uint8_t& z, const BlockID& block, const int8_t& type) {
				if (chunks[chunk].IndexCount > MaxFaceCount * 6) { WC_ERROR("Memory overflow!"); return; }
				
				glm::vec3 pos = { x , y, z };

				uint32_t texture = blockData[block].texture[(uint32_t)face.texID];

				uint8_t textureSizeX = 1;
				uint8_t textureSizeY = 1;
				
				glm::vec2 TexCoords[4] = {
					glm::vec2(0.f, 0.f),
					glm::vec2(0.f,          textureSizeY),
					glm::vec2(textureSizeX, textureSizeY),
					glm::vec2(textureSizeX, 0.f),
				};

				chunkMesh[    offset] = gl::Vertex(face.corner1 + pos, { TexCoords[0], texture }, type);
				chunkMesh[1 + offset] = gl::Vertex(face.corner2 + pos, { TexCoords[1], texture }, type);
				chunkMesh[2 + offset] = gl::Vertex(face.corner3 + pos, { TexCoords[2], texture }, type);
				chunkMesh[3 + offset] = gl::Vertex(face.corner4 + pos, { TexCoords[3], texture }, type);
				
				chunks[chunk].IndexCount += 6;
				offset += 4;
			};

			uint8_t y = 0, x = 0, z = 0; // @TODO: elif
					for (; y < chunkSize; y++)
					for (x = 0; x < chunkSize; x++)
					for (z = 0; z < chunkSize; z++)
					{
						BlockID block = chunks[chunk].chunkData[x][y][z];
						if (block > 0)
						{
							BlockID checkBlock = 0;
							ConnectionType type = blockData[block].blockConnectionType;

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
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(TOP_FACE, x, y, z, block, 0);
									}
									else if (chunks[chunk].neighborYpos >= 0) {
										checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(TOP_FACE, x, y, z, block, 0);
									}

									if (z + 1 < chunkSize) {
										checkBlock = chunks[chunk].chunkData[x][y][z + 1];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(FRONT_FACE, x, y, z, block, 0);
									}
									else if (chunks[chunk].neighborZpos >= 0) {
										checkBlock = chunks[chunks[chunk].neighborZpos].chunkData[x][y][0];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(FRONT_FACE, x, y, z, block, 0);
									}

									if (x + 1 < chunkSize) {
										checkBlock = chunks[chunk].chunkData[x + 1][y][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(RIGHT_FACE, x, y, z, block, 0);
									}
									else if (chunks[chunk].neighborXpos >= 0) {
										BlockID checkBlock = chunks[chunks[chunk].neighborXpos].chunkData[0][y][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(RIGHT_FACE, x, y, z, block, 0);
									}
									//Negative	   																										 					  															    						 
									if (y - 1 >= 0) {
										checkBlock = chunks[chunk].chunkData[x][y - 1][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(BOTTOM_FACE, x, y, z, block, 0);
									}
									else if (chunks[chunk].neighborYneg >= 0) {
										checkBlock = chunks[chunks[chunk].neighborYneg].chunkData[x][chunkSizeMinusOne][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(BOTTOM_FACE, x, y, z, block, 0);
									}

									if (z - 1 >= 0) {
										checkBlock = chunks[chunk].chunkData[x][y][z - 1];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(BACK_FACE, x, y, z, block, 0);
									}
									else if (chunks[chunk].neighborZneg >= 0) {
										checkBlock = chunks[chunks[chunk].neighborZneg].chunkData[x][y][chunkSizeMinusOne];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(BACK_FACE, x, y, z, block, 0);
									}

									if (x - 1 >= 0) {
										checkBlock = chunks[chunk].chunkData[x - 1][y][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(LEFT_FACE, x, y, z, block, 0);
									}
									else if (chunks[chunk].neighborXneg >= 0) {
										checkBlock = chunks[chunks[chunk].neighborXneg].chunkData[chunkSizeMinusOne][y][z];
										if (checkBlock == 0 && type != blockData[checkBlock].blockConnectionType)
											addFace(LEFT_FACE, x, y, z, block, 0);
									}
								}

							else if (type == ConnectionType::FLUID_CONNECT) // Can make a fluid face
							{
								//Positive
								if (y + 1 < chunkSize) {
									checkBlock = chunks[chunk].chunkData[x][y + 1][z];
									if (type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, x, y, z, block, 1);
								}
								else if (chunks[chunk].neighborYpos >= 0) {
									checkBlock = chunks[chunks[chunk].neighborYpos].chunkData[x][0][z];
									if (type != blockData[checkBlock].blockConnectionType)
										addFace(TOP_FACE, x, y, z, block, 1);
								}
							}

							else if (type == ConnectionType::X_CONNECT) // Can make a fluid face
							{
								addFace(X_FACE1, x, y, z, block, 2);
								addFace(X_FACE2, x, y, z, block, 2);
							}
						}
					}
			if (chunks[chunk].IndexCount > 0) chunks[chunk].chunkMeshBuffer.Update(0, sizeof(chunkMesh), &chunkMesh);
		}

		BlockID getBlock(const glm::ivec3& pos) {
			ChunkID chunk = getChunkID(getChunkPos(pos));
			glm::ivec3 blockPos = getBlockPos(pos);
			return chunks[chunk].chunkData[blockPos.x][blockPos.y][blockPos.z];
		}		

		int16_t getChunkID(const glm::ivec3& pos) {
			//@TODO: improve
			for (ChunkID i = 0; i < chunks.size(); i++) {
				if (chunks[i].chunkPos.x == pos.x && chunks[i].chunkPos.y == pos.y && chunks[i].chunkPos.z == pos.z) 
					return i;				
			}
			return -1;
		}
	};	
}
#endif