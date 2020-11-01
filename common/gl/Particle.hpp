#pragma once

#include <Utils/CustomDefs.hpp>
#include <Utils/Random.hpp>

#include <gl/Shaders.hpp>
#include <gl/VertexBuffer.hpp>
#include <gl/IndexBuffer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <vector>

struct ParticleProps
{
	glm::vec3 Position;
	glm::vec3 Velocity, VelocityVariation;
	glm::vec4 ColorBegin, ColorEnd;
	float SizeBegin, SizeEnd, SizeVariation;
	float LifeTime = 1.0f;
};


class ParticleEffect {
	ParticleEffect() {
		m_ParticlePool.resize(1000);
	}
	~ParticleEffect() {

	}

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
	void Render() {
		if (!m_QuadVA.GetVAO())
		{
			float vertices[] = {
				 -0.5f, -0.5f, 0.0f,
				  0.5f, -0.5f, 0.0f,
				  0.5f,  0.5f, 0.0f,
				 -0.5f,  0.5f, 0.0f
			};

			m_QuadVA.Create(vertices, sizeof(vertices)); // TODO
			gl::VertexAttribPointer(0, 3, 3 * sizeof(float), 0);

			uint32_t indices[] = {0, 1, 2, 2, 3, 0};

			gl::IndexBuffer quadIB(indices, sizeof(indices));

			m_ParticleShader.Create("");
			m_ParticleShaderViewProj = glGetUniformLocation(m_ParticleShader.GetRendererID(), "u_ViewProj");
			m_ParticleShaderTransform = glGetUniformLocation(m_ParticleShader.GetRendererID(), "u_Transform");
			m_ParticleShaderColor = glGetUniformLocation(m_ParticleShader.GetRendererID(), "u_Color");
		}

		m_ParticleShader.use();
		//glUniformMatrix4fv(m_ParticleShaderViewProj, 1, GL_FALSE, glm::value_ptr(camera.GetViewProjectionMatrix()));
		//m_ParticleShader.setMat4();

		for (auto& particle : m_ParticlePool)
		{
			if (!particle.Active) continue;

			// Fade away particles
			float life = particle.LifeRemaining / particle.LifeTime;
			glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
			//color.a = color.a * life;

			float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

			// Render
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, 0.0f })
				* glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f })
				* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });
			glUniformMatrix4fv(m_ParticleShaderTransform, 1, GL_FALSE, glm::value_ptr(transform));
			glUniform4fv(m_ParticleShaderColor, 1, glm::value_ptr(color));
			m_QuadVA.Bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		}
	}

	void Emit(const ParticleProps& particleProps) {
		Particle& particle = m_ParticlePool[m_PoolIndex];
		particle.Active = true;
		particle.Position = particleProps.Position;
		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		// Velocity
		particle.Velocity = particleProps.Velocity;
		particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);
		particle.Velocity.z += particleProps.VelocityVariation.z * (Random::Float() - 0.5f);

		// Color
		particle.ColorBegin = particleProps.ColorBegin;
		particle.ColorEnd = particleProps.ColorEnd;

		particle.LifeTime = particleProps.LifeTime;
		particle.LifeRemaining = particleProps.LifeTime;
		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = particleProps.SizeEnd;

		m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	}

private:	
struct Particle
{
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec4 ColorBegin, ColorEnd;
	float Rotation = 0.0f;
	float SizeBegin, SizeEnd;

	float LifeTime = 1.0f;
	float LifeRemaining = 0.0f;

	bool Active = false;
};
std::vector<Particle> m_ParticlePool;
uint32_t m_PoolIndex = 999;

gl::VertexBuffer m_QuadVA;
gl::Shader m_ParticleShader;
int m_ParticleShaderViewProj, m_ParticleShaderTransform, m_ParticleShaderColor;

};