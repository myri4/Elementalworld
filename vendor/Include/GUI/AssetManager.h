#pragma once
#include <gl/TextureArray.h>

namespace wc {
class AssetManager {
public:
	AssetManager() {}
	void Create(const uint32_t& arraySize) {
		const uint32_t width = 128;
		const uint32_t height = 128;
		texArr.Create(arraySize, width, height);
		textureMaterialArr.Create(arraySize, width, height);

		uint32_t* data = new uint32_t[width * height];

		for (uint32_t i = 0; i < width * height; i++) 
			data[i] = 0xFF000000;
		
		texArr.AddTexture(data);
		textureMaterialArr.AddTexture(data);
		delete[] data;
	}

	uint32_t LoadTexture(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		uint32_t location = 0;

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			location = texArr.AddTexture(data);
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	uint32_t LoadTextureMaterial(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		uint32_t location = 0;

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			location = textureMaterialArr.AddTexture(data);
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	void Free() {
		//for (const auto& [key, value] : m_TextureCache) WC_INFO("{0} {1}", key.c_str(), value);
		
		m_TextureCache.clear();
		texArr.GenerateMipmap();
		textureMaterialArr.GenerateMipmap();
	}

	void Bind() { 
		texArr.Bind(0); 
		textureMaterialArr.Bind(1);
	}
private:
	std::unordered_map<std::string, int> m_TextureCache;
	gl::TextureArray texArr;
	gl::TextureArray textureMaterialArr;
}assets;
}