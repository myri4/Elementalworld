#ifndef CUSTOM_DEFS_HPP
#define CUSTOM_DEFS_HPP

#include <memory>
#include <glm/glm.hpp>

//Custom definitions
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

//World defs
using ChunkID = uint16_t; // This represents the chunk id in the chunk array
using ChunkPos = int32_t; // This represents the chunk position in the 3D world

using Face = std::array < glm::vec3, 4 >;

using BlockID = uint8_t;
#endif