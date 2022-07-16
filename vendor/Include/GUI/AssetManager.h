#pragma once
#include <gl/TextureArray.h>

namespace wc {
class AssetManager {
public:
	AssetManager() {}
	void Create(const uint32_t& arraySize, const uint32_t& width, const uint32_t& height) { 
		texArr.Create(arraySize, width, height);
		modelTexArr.Create(arraySize, 128, 128);

		textureMaterialArr.Create(arraySize, width, height);
		modelTextureMarterialArray.Create(arraySize, 128, 128);


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

	uint32_t LoadTextureMaterial(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		uint32_t location = 0;

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			textureMaterialArr.AddTexture(data);
			location = textureMaterialArr.GetGeneretedTextures() - 1;
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	uint32_t LoadModelTexture(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist
		
		uint32_t location = 0;
		
		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			modelTexArr.AddTexture(data);
			location = modelTexArr.GetGeneretedTextures() - 1;
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	uint32_t LoadModelTextureMaterial(const std::string& file)
	{
		if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

		uint32_t location = 0;

		int fnrComponents = 0, fwidth = 0, fheight = 0;
		auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
		if (data) {
			modelTextureMarterialArray.AddTexture(data);
			location = modelTextureMarterialArray.GetGeneretedTextures() - 1;
			m_TextureCache[file] = location;
		}
		else WC_ERROR("Cannot find file at location: {0}", file);
		return location;
	}

	void Free() {
		m_TextureCache.clear();
		texArr.GenerateMipmap();
		modelTexArr.GenerateMipmap();
		textureMaterialArr.GenerateMipmap();
		modelTextureMarterialArray.GenerateMipmap();
	}

	void Bind() { 
		texArr.Bind(0); 
		textureMaterialArr.Bind(1);
	}

	void BindModelData() {
		modelTexArr.Bind(0);
		modelTextureMarterialArray.Bind(1);
	}
private:
	std::unordered_map<std::string, int> m_TextureCache;
	gl::TextureArray texArr;
	gl::TextureArray modelTexArr;

	gl::TextureArray textureMaterialArr;
	gl::TextureArray modelTextureMarterialArray;
}assets;
}