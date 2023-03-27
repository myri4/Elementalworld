#pragma once
#include <wc/vk/Images.h>
#include <filesystem>

namespace wc {

	std::string AssetPackName = "default";

	std::string GetAssetPath() { return "resourcepacks/" + AssetPackName; }
	std::string GetCachedAssetPath() { return "cache/" + AssetPackName; }

	namespace AssetManager {
		namespace {
			std::unordered_map<std::string, uint32_t> m_Cache; // This becomes obsolete after we transition to pre-baked asset packs
		}

		std::vector<wc::Texture> m_Textures;

		void Init() {
			wc::Texture texture;
			glm::vec3 white = glm::vec3(1.f);
			texture.Load(&white, 1, 1, false, false);
			m_Textures.push_back(texture);
		}
	
		uint32_t LoadTexture(const std::string& file)
		{
			if (m_Cache.find(file) != m_Cache.end()) return m_Cache[file];  // If this texture exist
			if (std::filesystem::exists(file)) {
				wc::Texture texture;
				texture.Load(file, true, false);
				m_Textures.push_back(texture);

				m_Cache[file] = m_Textures.size() - 1;
				return m_Textures.size() - 1;
			}
	
			m_Cache[file] = 0;
			WC_ERROR("Cannot find file at location: {0}", file);
			return 0;/*@TODO: Return some kind of debug texture to indicate that the texture is missing*/
		}

		void Destroy() {
			for (auto& texture : m_Textures) texture.Destroy();
		}
	}
}