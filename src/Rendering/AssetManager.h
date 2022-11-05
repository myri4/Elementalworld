#pragma once
#include <wc/vk/Images.h>
#include <filesystem>

namespace wc {

	std::string AssetPackName = "default";

	std::string GetAssetPath() { return "resourcepacks/" + AssetPackName; }
	std::string GetCachedAssetPath() { return "cache/" + AssetPackName; }

	class AssetManager {
	public:
		AssetManager() {}
	
		uint32_t LoadTexture(const std::string& file)
		{
			if (m_DiffuseCache.find(file) != m_DiffuseCache.end()) return m_DiffuseCache[file];  // If this texture exist
			if (std::filesystem::exists(file)) {
				m_DiffuseCache[file] = textureCounter;
				textureCounter++;
				return textureCounter - 1;
			}
	
			m_DiffuseCache[file] = -1;
			WC_ERROR("Cannot find file at location: {0}", file);
			return 0;/*@TODO: Return some kind of debug texture to indicate that the texture is missing*/
		}
	
		uint32_t LoadTextureMaterial(const std::string& file)
		{
			if (m_MaterialCache.find(file) != m_MaterialCache.end()) return m_MaterialCache[file];  // If this texture exist
			if (std::filesystem::exists(file)) {
				m_MaterialCache[file] = materialCounter;
				materialCounter++;
				return materialCounter - 1;
			}
	
			m_MaterialCache[file] = -1;
			WC_ERROR("Cannot find file at location: {0}", file);
			return 0;/*@TODO: Return some kind of debug texture to indicate that the texture is missing*/
		}
	
		void LoadAll() {
	
			{
				const uint32_t width = 128;
				const uint32_t height = 128;
				texArr.Create({ width, height , textureCounter });
				textureMaterialArr.Create({ width, height , materialCounter });
	
				uint32_t* data = new uint32_t[width * height];
	
				for (uint32_t i = 0; i < width * height; i++)
					data[i] = 0xFFFFFFFF;
	
				texArr.SetTextureData(data, 0);
				delete[] data;
			}
	
			VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	
			sampler.Create(samplerInfo);
	
			for (auto& [file, id] : m_DiffuseCache) {
				if (id != -1) {
					int fnrComponents = 0, fwidth = 0, fheight = 0;
					auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
	
					texArr.SetTextureData(data, id);
					stbi_image_free(data);
				}
			}
	
			for (auto& [file, id] : m_MaterialCache) {
				if (id != -1) {
					int fnrComponents = 0, fwidth = 0, fheight = 0;
					auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
	
					textureMaterialArr.SetTextureData(data, id);
					stbi_image_free(data);
				}
			}
	
			m_DiffuseCache.clear();
			m_MaterialCache.clear();
			//texArr.GenerateMipmap();
			//textureMaterialArr.GenerateMipmap();
		}
	
		void Destroy() {
			texArr.Destroy();
			textureMaterialArr.Destroy();
			sampler.Destroy();
		}
	
		wc::TextureArray texArr;
		wc::TextureArray textureMaterialArr;
		wc::Sampler sampler;
	private:
		std::unordered_map<std::string, int> m_DiffuseCache;
		std::unordered_map<std::string, int> m_MaterialCache;
		uint32_t textureCounter = 1;
		uint32_t materialCounter = 1;
	}assets;
}