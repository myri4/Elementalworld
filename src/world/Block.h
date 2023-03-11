#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <wc/Maths/AssimpGLMHelpers.h>
#include <wc/Utils/Log.h>
#include "../Game Mechanics/Item.h"
#include <magic_enum.hpp>
#include <wc/Utils/List.h>
#include <wc/Utils/YAML.h>
#include "../Rendering/AssetManager.h"
#include "../Rendering/Renderer3D.h"
#include "../Globals.h"

namespace wc {

	struct Block {
		bool emitLight : 1;
		ConnectionType connectionType = ConnectionType::AIR;

		bool isCollidable : 1;
		MeshID meshID = 0;
		uint8_t variations = 0;
		MaterialID materialIDs[6] = { 0 };
		glm::vec3 rotation = glm::vec3(0.f);

		uint32_t flags = 0;

		std::string name = "air";

		Block() {
			isCollidable = false;
			emitLight = false;
		}
	};

	List<Block, 40> blockData;
}