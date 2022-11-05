#pragma once

#include <glm/glm.hpp>

#include <wc/vk/Buffer.h>
#include <wc/vk/Images.h>
#include <wc/Shader.h>
#include <wc/vk/Renderpass.h>
#include <wc/vk/RendererContext.h>

#include <glm/gtc/type_ptr.hpp>
#include "AssetManager.h"

namespace wc {

	class RenderInterface
	{
		wc::Shader shader;
	public:
		void Create(const wc::RenderPass& renderPass, const VkDescriptorImageInfo& info, const glm::vec2& windowSize) {

			wc::ShaderCreateInfo createInfo;
			createInfo.vertexShader   = GetAssetPath() + "/shaders/RmlRenderer.vert";
			createInfo.fragmentShader = GetAssetPath() + "/shaders/RmlRenderer.frag";
			createInfo.cachePath = GetCachedAssetPath() + "/shaders/Renderer2D.bin";
			createInfo.windowSize = windowSize;
			createInfo.renderPass = renderPass;
			createInfo.blending = true;
			createInfo.depthTest = false;
			createInfo.invertY = true;
			shader.Create(createInfo);

			VkWriteDescriptorSet newWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			newWrite.descriptorCount = 1;
			newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			newWrite.pImageInfo = &info;
			newWrite.dstBinding = 0;
			newWrite.dstSet = shader.descriptorSet;

			wc::UpdateDescriptorSets(1, &newWrite);
		}

		void Destroy() {
			shader.Destroy();
		}

		void Flush() {
			wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;

			shader.Bind(cmd);
			cmd.Draw(3);
		}
	} render_interface;
}