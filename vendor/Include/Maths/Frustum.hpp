#pragma once
#include <glm/matrix.hpp>
#include "Camera.hpp"

namespace wc {

    struct AABB {

        AABB() {}
        AABB(const glm::vec3& pos, const glm::vec3& dims) : position(pos), size(dims) {}

        glm::vec3 getVN(const glm::vec3& normal) const
        {
            glm::vec3 res = position;

            if (normal.x < 0) res.x += size.x;
            if (normal.y < 0) res.y += size.y;
            if (normal.z < 0) res.z += size.z;

            return res;
        }

        glm::vec3 getVP(const glm::vec3& normal) const
        {
            glm::vec3 res = position;

            if (normal.x > 0) res.x += size.x;
            if (normal.y > 0) res.y += size.y;
            if (normal.z > 0) res.z += size.z;

            return res;
        }

        glm::vec3 position = glm::vec3(0.f);
        glm::vec3 size = glm::vec3(0.f);
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

        bool isBoxInFrustum(const AABB& box) const noexcept
        {
            bool result = true;
            for (auto& plane : m_planes) {
                if (plane.distanceToPoint(box.getVP(plane.normal)) < 0.f)
                {
                    return false;
                }
                else if (plane.distanceToPoint(box.getVN(plane.normal)) < 0.f)
                    result = true;

            }
            return result;
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
                return glm::dot(point, normal) + distance;
            }

            glm::vec3 normal;
            float distance;
        } m_planes[6];
    };
}