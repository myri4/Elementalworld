#pragma once
#include <glm/matrix.hpp>
#include "Camera.hpp"

namespace wc {

struct AABB
{
    glm::vec3 center{ 0.f, 0.f, 0.f };
    glm::vec3 extents{ 0.f, 0.f, 0.f };

    AABB(const glm::vec3& min, const glm::vec3& max)
        : center{ (max + min) * 0.5f },
        extents{ max.x - center.x, max.y - center.y, max.z - center.z }
    {}

    AABB(const glm::vec3& inCenter, float iI, float iJ, float iK)
        : center{ inCenter }, extents{ iI, iJ, iK }
    {}
};

class Frustum {
public:
    void update(const glm::mat4& mat) noexcept
    {
        // left
        m_planes[2].normal.x = mat[0][3] + mat[0][0];
        m_planes[2].normal.y = mat[1][3] + mat[1][0];
        m_planes[2].normal.z = mat[2][3] + mat[2][0];
        m_planes[2].distance = mat[3][3] + mat[3][0];

        // right
        m_planes[3].normal.x = mat[0][3] - mat[0][0];
        m_planes[3].normal.y = mat[1][3] - mat[1][0];
        m_planes[3].normal.z = mat[2][3] - mat[2][0];
        m_planes[3].distance = mat[3][3] - mat[3][0];

        // bottom
        m_planes[5].normal.x = mat[0][3] + mat[0][1];
        m_planes[5].normal.y = mat[1][3] + mat[1][1];
        m_planes[5].normal.z = mat[2][3] + mat[2][1];
        m_planes[5].distance = mat[3][3] + mat[3][1];

        // top
        m_planes[4].normal.x = mat[0][3] - mat[0][1];
        m_planes[4].normal.y = mat[1][3] - mat[1][1];
        m_planes[4].normal.z = mat[2][3] - mat[2][1];
        m_planes[4].distance = mat[3][3] - mat[3][1];

        // near
        m_planes[0].normal.x = mat[0][3] + mat[0][2];
        m_planes[0].normal.y = mat[1][3] + mat[1][2];
        m_planes[0].normal.z = mat[2][3] + mat[2][2];
        m_planes[0].distance = mat[3][3] + mat[3][2];

        // far
        m_planes[1].normal.x = mat[0][3] - mat[0][2];
        m_planes[1].normal.y = mat[1][3] - mat[1][2];
        m_planes[1].normal.z = mat[2][3] - mat[2][2];
        m_planes[1].distance = mat[3][3] - mat[3][2];
    }

    void update(const Camera& cam, const float& aspect, const float& zNear, const float& zFar){
        const float halfVSide = zFar * glm::tan(cam.FOV * 0.5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * cam.Front;
    
        m_planes[0] = { cam.Position + zNear * cam.Front, cam.Front };
        m_planes[1] = { cam.Position + frontMultFar, -cam.Front };
        m_planes[2] = { cam.Position, glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
        m_planes[3] = { cam.Position, glm::cross(cam.Up,frontMultFar + cam.Right * halfHSide) };
        m_planes[4] = { cam.Position, glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
        m_planes[5] = { cam.Position, glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right) };
    }

    bool isBoxInFrustum(const glm::vec3& center, const float& radius = 0.f) const {
        // Loop through each plane that comprises the frustum.
        for (uint8_t i = 0; i < 6; i++)
        {
            // Plane-sphere intersection test. If p*n + d + r < 0 then we're outside the plane.
            if (glm::dot(center, m_planes[i].normal) + m_planes[i].distance + radius <= 0.f)
                return false;
        }

        // If none of the planes had the entity lying on its "negative" side then it must be
        // on the "positive" side for all of them. Thus the entity is inside or touching the frustum.
        return true;
    }

private:
    struct Plane {
        Plane() = default;

        Plane(const glm::vec3& p1, const glm::vec3& norm)
            : normal(norm),
            distance(glm::dot(normal, p1))
        {}

        float distanceToPoint(const glm::vec3& point) const
        {
            return glm::dot(normal, point) - distance;
        }

        glm::vec3 normal;
        float distance;
    } m_planes[6];
};
}