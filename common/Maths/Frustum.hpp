#pragma once
#include <glm/matrix.hpp>

namespace wc {

class Frustum {
public:
    void update(const glm::mat4& mat) noexcept
    {
        // left
        m_planes[2].normal.x = mat[0][3] + mat[0][0];
        m_planes[2].normal.y = mat[1][3] + mat[1][0];
        m_planes[2].normal.z = mat[2][3] + mat[2][0];
        m_planes[2].distanceToOrigin = mat[3][3] + mat[3][0];

        // right
        m_planes[3].normal.x = mat[0][3] - mat[0][0];
        m_planes[3].normal.y = mat[1][3] - mat[1][0];
        m_planes[3].normal.z = mat[2][3] - mat[2][0];
        m_planes[3].distanceToOrigin = mat[3][3] - mat[3][0];

        // bottom
        m_planes[5].normal.x = mat[0][3] + mat[0][1];
        m_planes[5].normal.y = mat[1][3] + mat[1][1];
        m_planes[5].normal.z = mat[2][3] + mat[2][1];
        m_planes[5].distanceToOrigin = mat[3][3] + mat[3][1];

        // top
        m_planes[4].normal.x = mat[0][3] - mat[0][1];
        m_planes[4].normal.y = mat[1][3] - mat[1][1];
        m_planes[4].normal.z = mat[2][3] - mat[2][1];
        m_planes[4].distanceToOrigin = mat[3][3] - mat[3][1];

        // near
        m_planes[0].normal.x = mat[0][3] + mat[0][2];
        m_planes[0].normal.y = mat[1][3] + mat[1][2];
        m_planes[0].normal.z = mat[2][3] + mat[2][2];
        m_planes[0].distanceToOrigin = mat[3][3] + mat[3][2];

        // far
        m_planes[1].normal.x = mat[0][3] - mat[0][2];
        m_planes[1].normal.y = mat[1][3] - mat[1][2];
        m_planes[1].normal.z = mat[2][3] - mat[2][2];
        m_planes[1].distanceToOrigin = mat[3][3] - mat[3][2];

        for (int8_t i = 0; i < 6; i++)
            glm::normalize(m_planes[i].normal);
    }

    bool isBoxInFrustum(const glm::vec3& center, const float& radius = 0.0f) const {
        // Loop through each plane that comprises the frustum.
        for (int8_t i = 0; i < 6; i++)
        {
            // Plane-sphere intersection test. If p*n + d + r < 0 then we're outside the plane.
            if (glm::dot(center, m_planes[i].normal) + m_planes[i].distanceToOrigin + radius <= 0)
                return false;
        }

        // If none of the planes had the entity lying on its "negative" side then it must be
        // on the "positive" side for all of them. Thus the entity is inside or touching the frustum.
        return true;
    }

private:
    struct Plane {
        float distanceToPoint(const glm::vec3& point) const
        {
            return glm::dot(point, normal) + distanceToOrigin;
        }

        float distanceToOrigin;
        glm::vec3 normal;
    };

    Plane m_planes[6];
};
}