#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP
#include <gl/TextureArray.hpp>
#include <glm/glm.hpp>

namespace wc {
class AssetManager {
public:
	AssetManager() {}
	void Create(const uint32_t& arraySize, const uint32_t& width, const uint32_t& height, const uint8_t& nrOfComponents = 4) { texArr.Create(arraySize, width, height, nrOfComponents); }

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

	void Bind() { texArr.Bind(); }
	void BindNormal() { texArr.Bind(); }
	gl::Texture textures[5];
private:
	std::unordered_map<std::string, int> m_TextureCache;
	gl::TextureArray texArr;
	gl::TextureArray normalTexArr;
};
}

#endif
