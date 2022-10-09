#pragma once
#include <pch.h>
#include <wc/Maths/Frustum.h>


namespace wc {

	static const uint32_t MaxLineVertexCount = 100 * 2;

	struct LineVertex {
		glm::vec3 pos;
		glm::vec4 color;

		static wc::VertexInputDescription get_vertex_description() {
			wc::VertexInputDescription description;

			//we will have just 1 vertex buffer binding, with a per-vertex rate
			VkVertexInputBindingDescription mainBinding = {};
			mainBinding.binding = 0;
			mainBinding.stride = sizeof(LineVertex);
			mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			description.bindings.push_back(mainBinding);

			//Position will be stored at Location 0
			VkVertexInputAttributeDescription positionAttribute = {};
			positionAttribute.binding = 0;
			positionAttribute.location = 0;
			positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			positionAttribute.offset = offsetof(LineVertex, pos);

			//Normal will be stored at Location 1
			VkVertexInputAttributeDescription colorAttribute = {};
			colorAttribute.binding = 0;
			colorAttribute.location = 1;
			colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			colorAttribute.offset = offsetof(LineVertex, color);

			description.attributes.push_back(positionAttribute);
			description.attributes.push_back(colorAttribute);
			return description;
		}
	};

	class LineBatcher {
		uint32_t IndexCount = 0;
		uint32_t byteOffset = 0;

		wc::Buffer lineBuffer;
		wc::Shader shader;
	public:
		void Create(const wc::RenderPass& renderPass, const VkDescriptorBufferInfo& ubo) {
			wc::ShaderCreateInfo createInfo;
			createInfo.vertexShader = "resourcepacks/default/shaders/Line3D.vert";
			createInfo.fragmentShader = "resourcepacks/default/shaders/Line3D.frag";
			createInfo.windowSize = window.GetSize();
			createInfo.renderPass = renderPass;
			createInfo.vertexDescription = LineVertex::get_vertex_description();
			createInfo.blending = false;
			createInfo.depthTest = true;
			createInfo.invertY = true;
			createInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			shader.Create(createInfo);

			wc::DescriptorWriter writer;

			writer.dstSet = shader.descriptorSet;
			writer.write_buffer(0, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

			wc::UpdateDescriptorSets(writer.writes.size(), writer.writes.data());

			lineBuffer.Create(MaxLineVertexCount * sizeof(LineVertex), wc::VERTEX_BUFFER);
		}

		void Destroy() {
			lineBuffer.Destroy();
			shader.Destroy();
		}

		void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f)) {
			if (IndexCount >= MaxLineVertexCount) Flush();

			float vertices[] = {
				// positions
				start.x, start.y, start.z, color.r, color.g, color.b, color.a,
				end.x,   end.y,   end.z  , color.r, color.g, color.b, color.a
			};

			lineBuffer.SetData(vertices, sizeof(vertices), byteOffset);
			byteOffset += sizeof(vertices);
			IndexCount += 2;
		}

		void DrawOutlineCube(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color) {
			DrawLine(pos, pos + glm::vec3(0.f, size.y, 0.f), color);
			DrawLine(pos, pos + glm::vec3(size.x, 0.f, 0.f), color);
			DrawLine(pos + glm::vec3(size.x, 0.f, 0.f), pos + glm::vec3(size.x, size.y, 0.f), color);
			DrawLine(pos + glm::vec3(size.x, size.y, 0.f), pos + glm::vec3(0.f, size.y, 0.f), color);

			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos + glm::vec3(0.f, size.y, size.z), color);
			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos + glm::vec3(size.x, 0.f, size.z), color);
			DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, size.y, size.z), color);
			DrawLine(pos + size, pos + glm::vec3(0.f, size.y, size.z), color);

			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos, color);
			DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, 0.f, 0.f), color);

			DrawLine(pos + glm::vec3(0.f, size.y, size.z), pos + glm::vec3(0.f, size.y, 0.f), color);
			DrawLine(pos + size, pos + glm::vec3(size.x, size.y, 0.f), color);
		}

		void DrawOutlineCube(const AABB& aabb, const glm::vec4& color) {
			DrawOutlineCube(aabb.position, aabb.size, color);
		}

		void Flush(const bool render = true) {
			if (!IndexCount) return;
			if (render) {
				wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;
				shader.Bind(cmd);
				cmd.BindVertexBuffer(lineBuffer);
				cmd.Draw(IndexCount);
			}
			byteOffset = 0;
			IndexCount = 0;
		}

		LineBatcher() {	}
	};
}