#pragma once
#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>
#include <Utils/Random.hpp>

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

namespace wc {

	const uint32_t MaxParticleCount = 999;

	struct ParticleVertex {
		glm::vec3 pos;
		glm::vec4 color; // @Temp
	}; 
	
	struct ParticleProps
	{
		glm::vec3 Position;
		glm::vec3 Velocity, VelocityVariation;
		glm::vec4 ColorBegin, ColorEnd;
		float SizeBegin, SizeEnd, SizeVariation;
		float LifeTime = 1.f;
	};

	class ParticleSystem {
	private: // Variables

		Random random;		

		struct Particle
		{
			glm::vec3 Position;
			glm::vec3 Velocity;
			glm::vec4 ColorBegin, ColorEnd;
			float Rotation = 0.f;
			float SizeBegin, SizeEnd;

			float LifeTime = 1.f;
			float LifeRemaining = 0.f;

			bool Active = false;
		};

		gl::VertexArray vParticleArray;
		gl::Shader shader;

		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex = 999;

	public:

		void Create() {
			m_ParticlePool.resize(1000);
			shader.Create("shaderpacks/default/particleShader.glsl");

			float vertices[] = {
			 -0.5f, -0.5f, 0.0f,
			  0.5f, -0.5f, 0.0f,
			  0.5f,  0.5f, 0.0f,
			 -0.5f,  0.5f, 0.0f
			};

			uint32_t indices[] = {
				0, 1, 2, 2, 3, 0
			};

			vParticleArray.Create();
			gl::IndexBuffer iParticleBuffer;
			gl::VertexBuffer vParticleBuffer;
			vParticleBuffer.Create(vertices, sizeof(vertices));
			Renderer::VertexAttribPointer(0, 3, sizeof(ParticleVertex), (const void*)offsetof(ParticleVertex, pos));
			iParticleBuffer.Create(indices, sizeof(indices));
			//Renderer::VertexAttribPointer(1, 4, sizeof(ParticleVertex), (const void*)offsetof(ParticleVertex, color));

			random.Init();
		}

		void Emit(const ParticleProps& particleProps) {
			Particle& particle = m_ParticlePool[m_PoolIndex];
			particle.Active = true;
			particle.Position = particleProps.Position;
			particle.Rotation = random.Float() * 2.f * glm::pi<float>();

			// Velocity
			particle.Velocity = particleProps.Velocity;
			particle.Velocity.x += particleProps.VelocityVariation.x * (random.Float() - 0.5f);
			particle.Velocity.y += particleProps.VelocityVariation.y * (random.Float() - 0.5f);

			// Color
			particle.ColorBegin = particleProps.ColorBegin;
			particle.ColorEnd = particleProps.ColorEnd;

			particle.LifeTime = particleProps.LifeTime;
			particle.LifeRemaining = particleProps.LifeTime;
			particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (random.Float() - 0.5f);
			particle.SizeEnd = particleProps.SizeEnd;

			m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
		}

		void OnUpdate(const float& deltaTime) {
			for (auto& particle : m_ParticlePool)
			{
				if (!particle.Active)
					continue;

				if (particle.LifeRemaining <= 0.0f)
				{
					particle.Active = false;
					continue;
				}

				particle.LifeRemaining -= deltaTime;
				particle.Position += particle.Velocity * deltaTime;
				particle.Rotation += 0.01f * deltaTime;
			}
		}

		void Render(const glm::mat4& view, const glm::mat4& proj) {
			for (auto& particle : m_ParticlePool)
			{
				if (!particle.Active)
					continue;

				// Fade away particles
				float life = particle.LifeRemaining / particle.LifeTime;
				glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);

				float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

				// Render
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), particle.Position) /* glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f }) */ * glm::scale(glm::mat4(1.0f), { size, size, size });
				shader.setMat4("u_View", view);
				shader.setMat4("u_Projection", proj);
				shader.setMat4("u_Model", transform);
				shader.setVec4("color", color);
				vParticleArray.Bind();
				Renderer::DrawIndexed(6);
			}
		}

		ParticleSystem() {}
	};
}