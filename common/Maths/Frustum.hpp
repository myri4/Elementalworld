#pragma once

#include <glm/glm.hpp>
#include <array>
#include "../../src/world/Chunk.hpp"

namespace wc {

struct Plane {
	float distanceToPoint(const glm::vec3& point) const
	{
		return glm::dot(point, normal) + distanceToOrigin;
	}

	float distanceToOrigin;
	glm::vec3 normal;
};

enum class Planes {
    Near,
    Far,
    Left,
    Right,
    Top,
    Bottom,
};

class ViewFrustum {
public:
	ViewFrustum() {}
    void update(const glm::mat4& mat) noexcept
    {
        // left
        m_planes[(int)Planes::Left].normal.x = mat[0][3] + mat[0][0];
        m_planes[(int)Planes::Left].normal.y = mat[1][3] + mat[1][0];
        m_planes[(int)Planes::Left].normal.z = mat[2][3] + mat[2][0];
        m_planes[(int)Planes::Left].distanceToOrigin = mat[3][3] + mat[3][0];

        // right
        m_planes[(int)Planes::Right].normal.x = mat[0][3] - mat[0][0];
        m_planes[(int)Planes::Right].normal.y = mat[1][3] - mat[1][0];
        m_planes[(int)Planes::Right].normal.z = mat[2][3] - mat[2][0];
        m_planes[(int)Planes::Right].distanceToOrigin = mat[3][3] - mat[3][0];

        // bottom
        m_planes[(int)Planes::Bottom].normal.x = mat[0][3] + mat[0][1];
        m_planes[(int)Planes::Bottom].normal.y = mat[1][3] + mat[1][1];
        m_planes[(int)Planes::Bottom].normal.z = mat[2][3] + mat[2][1];
        m_planes[(int)Planes::Bottom].distanceToOrigin = mat[3][3] + mat[3][1];

        // top
        m_planes[(int)Planes::Top].normal.x = mat[0][3] - mat[0][1];
        m_planes[(int)Planes::Top].normal.y = mat[1][3] - mat[1][1];
        m_planes[(int)Planes::Top].normal.z = mat[2][3] - mat[2][1];
        m_planes[(int)Planes::Top].distanceToOrigin = mat[3][3] - mat[3][1];
                 
        // near  
        m_planes[(int)Planes::Near].normal.x = mat[0][3] + mat[0][2];
        m_planes[(int)Planes::Near].normal.y = mat[1][3] + mat[1][2];
        m_planes[(int)Planes::Near].normal.z = mat[2][3] + mat[2][2];
        m_planes[(int)Planes::Near].distanceToOrigin = mat[3][3] + mat[3][2];
                 
        // far   
        m_planes[(int)Planes::Far].normal.x = mat[0][3] - mat[0][2];
        m_planes[(int)Planes::Far].normal.y = mat[1][3] - mat[1][2];
        m_planes[(int)Planes::Far].normal.z = mat[2][3] - mat[2][2];
        m_planes[(int)Planes::Far].distanceToOrigin = mat[3][3] - mat[3][2];

        for (auto& plane : m_planes) {
            float length = glm::length(plane.normal);
            plane.normal /= length;
            plane.distanceToOrigin /= length;
        }
    }
    bool isBoxInFrustum(const int32_t& chunk) const noexcept
    {
        glm::vec3 pos = to3D(chunk);
        for (auto& plane : m_planes) {
            if (plane.distanceToPoint(pos) <= 0) {
                return false;
            }
        }
        return true;
    }
private:
	std::array<Plane, 6> m_planes;
};
}