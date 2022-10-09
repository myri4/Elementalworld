#pragma once
#include "../vk/Images.h"

namespace wc {

class AssetManager {
public:
	AssetManager() {}
	void Create(const uint32_t& arraySize) {
		const uint32_t width = 128;
		const uint32_t height = 128;
		texArr.Create({ width, height ,arraySize });
		textureMaterialArr.Create({ width, height ,arraySize });

		uint32_t* data = new uint32_t[width * height];

		for (uint32_t i = 0; i < width * height; i++)
			data[i] = 0xFFFFFFFF;

		texArr.SetTextureData(data, 0);

		VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

		sampler.magFilter = VK_FILTER_NEAREST;
		sampler.minFilter = VK_FILTER_NEAREST;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		texArr.SetSamplerInfo(sampler);
		textureMaterialArr.SetSamplerInfo(sampler);
		delete[] data;
	}

	uint32_t LoadTexture(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		uint32_t location = textureCounter;
		if (data) {
			texArr.SetTextureData(data, textureCounter);
			m_TextureCache[file] = textureCounter;
			textureCounter++;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	uint32_t LoadTextureMaterial(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		uint32_t location = materialCounter;
		if (data) {
			textureMaterialArr.SetTextureData(data, materialCounter);
			m_TextureCache[file] = materialCounter;
			materialCounter++;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	void Free() {
		//for (const auto& [key, value] : m_TextureCache) WC_INFO("{0} {1}", key.c_str(), value);

		m_TextureCache.clear();
		//texArr.GenerateMipmap();
		//textureMaterialArr.GenerateMipmap();
	}

	void Destroy() {
		texArr.Destroy();
		textureMaterialArr.Destroy();
	}
	
	wc::TextureArray texArr;
	wc::TextureArray textureMaterialArr;
private:
	std::unordered_map<std::string, int> m_TextureCache;
	uint32_t textureCounter = 1;
	uint32_t materialCounter = 1;
}assets;
}