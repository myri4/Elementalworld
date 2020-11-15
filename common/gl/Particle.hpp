#pragma once

#include <Utils/CustomDefs.hpp>
#include <Utils/Random.hpp>

#include <gl/Shaders.hpp>
#include <gl/Vertex.hpp>
#include <gl/IndexBuffer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>
#include <vector>


namespace gl {
struct ParticleProps
{
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Velocity = glm::vec3(1.0f), VelocityVariation = glm::vec3(1.0f);
	glm::vec4 ColorBegin = glm::vec4(1.0f), ColorEnd = glm::vec4(1.0f);
	float SizeBegin = 1.0f, SizeEnd = 0.0f, SizeVariation = 1.0f;
	float LifeTime = 1.0f;
};


class ParticleEffect {
public:
	ParticleEffect() {}

	void OnUpdate(const float& deltaTime) {
		for (auto & particle : m_ParticlePool)
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
	void Render(const glm::mat4& proj) {
		if (!m_QuadVA.GetVAO())
		{
			float vertices[] = {
				 -0.5f,  0.5f,  0.0f,
				 -0.5f, -0.5f,  0.0f,
				  0.5f, -0.5f,  0.0f,
				  0.5f,  0.5f,  0.0f,
			};

			m_QuadVA.Create(vertices, sizeof(vertices)); // TODO
			gl::VertexAttribPointer(0, 3, 3 * sizeof(float), 0);

			uint32_t indices[] = {0, 1, 2, 2, 3, 0};

			gl::IndexBuffer quadIB(indices, sizeof(indices));

			m_ParticleShader.Create("shaderpacks/default/particleShader.glsl");
		}

		m_ParticleShader.use();
		m_ParticleShader.setMat4("u_Projection", proj);

		for (auto& particle : m_ParticlePool)
		{
			if (!particle.Active) continue;

			// Fade away particles
			float life = particle.LifeRemaining / particle.LifeTime;
			glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);

			float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

			// Render
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), particle.Position) * glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), glm::vec3(size));
			m_ParticleShader.setMat4("u_Model", transform);
			m_ParticleShader.setVec4("u_Color",color);
			m_QuadVA.BindVAO();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		}
	}

	void Emit(const ParticleProps& particleProps) {
		m_ParticlePool[m_PoolIndex].Active = true;
		m_ParticlePool[m_PoolIndex].Position = particleProps.Position;
		m_ParticlePool[m_PoolIndex].Rotation = Random::Float() * 2.0f * glm::pi<float>();

		// Velocity
		m_ParticlePool[m_PoolIndex].Velocity = particleProps.Velocity;
		m_ParticlePool[m_PoolIndex].Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
		m_ParticlePool[m_PoolIndex].Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);
		m_ParticlePool[m_PoolIndex].Velocity.z += particleProps.VelocityVariation.z * (Random::Float() - 0.5f);

		// Color
		m_ParticlePool[m_PoolIndex].ColorBegin = particleProps.ColorBegin;
		m_ParticlePool[m_PoolIndex].ColorEnd = particleProps.ColorEnd;
		
		m_ParticlePool[m_PoolIndex].LifeTime = particleProps.LifeTime;
		//m_ParticlePool[m_PoolIndex].LifeRemaining = particleProps.LifeTime;
		m_ParticlePool[m_PoolIndex].SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		m_ParticlePool[m_PoolIndex].SizeEnd = particleProps.SizeEnd;

		m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	}

private:	
	struct Particle
	{
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Velocity = glm::vec3(1.0f);
		glm::vec4 ColorBegin = glm::vec4(1.0f), ColorEnd = glm::vec4(1.0f);
		float Rotation = 0.0f;
		float SizeBegin = 1.0f, SizeEnd = 0.0f;
	
		float LifeTime = 1.0f;
		float LifeRemaining = 0.0f;
	
		bool Active = false;
	};
	std::array<Particle, 1000> m_ParticlePool;
	uint32_t m_PoolIndex = 999;
	
	gl::VertexBuffer m_QuadVA;
	gl::Shader m_ParticleShader;
};
}