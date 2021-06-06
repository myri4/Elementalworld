#ifndef BONEINFO_HPP
#define BONEINFO_HPP

#include <glm/glm.hpp>

namespace wc {
struct BoneInfo {
	int id; //For uniquely indentifying the bone and for indexing bone transformation in shaders map from bone name to offset matrix.
	glm::mat4 offset; // offset matrix transforms bone from bone space to local space
};
}
#endif