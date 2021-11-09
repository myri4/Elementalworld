#pragma once
#include <gl/TextureArray.hpp>

namespace wc {
class AssetManager {
public:
	AssetManager() {}
	void Create(const uint32_t& arraySize, const uint32_t& width, const uint32_t& height, const uint8_t& nrOfComponents = 4) { 
		texArr.Create(arraySize, width, height, nrOfComponents);
		uint8_t* data = new uint8_t[width * height * 4];

		for (uint32_t i = 0; i < width * height * 4; i++) 
			data[i] = 0x0000FF;
		
		texArr.AddTexture(data);
		delete[] data;
	}

	uint32_t LoadTexture(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		uint32_t location = 0;

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			texArr.AddTexture(data);
			location = texArr.GetGeneretedTextures() - 1;
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	uint32_t LoadNormalTexture(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		uint32_t location = 0;

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			normalTexArr.AddTexture(data);
			location = normalTexArr.GetGeneretedTextures() - 1;
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	void Free() {
		m_TextureCache.clear();
	}

	void Bind(const uint32_t& unit = 0) { texArr.Bind(unit); }
	void BindNormal(const uint32_t& unit = 0) { texArr.Bind(unit); }
	gl::Texture textures[5];
private:
	std::unordered_map<std::string, int> m_TextureCache;
	gl::TextureArray texArr;
	gl::TextureArray normalTexArr;
}assets;

enum class MenuMode { GAME, INVENTORY };
MenuMode mode = MenuMode::GAME; // @TODO: Remove it from here and make a file that needs to be include everywhere
}