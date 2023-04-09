#pragma once

#include <glm/gtc/matrix_transform.hpp>

namespace wc{

	class Camera {
	public:
		// camera Attributes
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);

		// Ray tracing attributes
		glm::vec3 lower_left_corner = glm::vec3(0.f);
		glm::vec3 horizontal = glm::vec3(0.f);
		glm::vec3 vertical = glm::vec3(0.f);
		float distanceFromCamera = 0.f;
		// euler Angles
		float Yaw = 0.f;
		float Pitch = 0.f;
		float Roll = 90.f;
		// camera options
		float FOV = glm::radians(90.f);

		// constructor with vectors
		Camera() = default;

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

		void Update(float aspectRatio) {
			// update Front, Right and Up Vectors using the updated Euler angles
			// calculates the new Front vector from the Camera's (updated) Euler Angles
			float yaw = glm::radians(Yaw);
			float pitch = glm::radians(Pitch);

			Front.x = glm::cos(yaw) * glm::cos(pitch);
			Front.y = glm::sin(pitch);
			Front.z = glm::sin(yaw) * glm::cos(pitch);
			Front = glm::normalize(Front);

			//rotate(Yaw, glm::vec3(0.f, 1.f, 0.f));
			//rotate(Pitch, glm::vec3(1.f, 0.f, 0.f));
			//rotate(glm::radians(Roll), glm::vec3(0.f, 0.f, 1.f));

			glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
			//up.x = glm::cos(glm::radians(Roll));
			//up.y = glm::sin(glm::radians(Roll));
			//up = normalize(up);
			Position -= Front * distanceFromCamera;

			float theta = glm::tan(FOV * 0.5f);
			float viewport_height = 2.f * theta;
			float viewport_width = aspectRatio * viewport_height;

			glm::vec3 Right = normalize(cross(Front, up));  // u normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			Up = normalize(cross(Right, Front));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			horizontal = viewport_width * Right;
			vertical = viewport_height * Up;
			lower_left_corner = Position - horizontal * 0.5f - vertical * 0.5f + Front;
		}
	};

    struct FAABB {

        FAABB() {}
        FAABB(const glm::vec3& pos, const glm::vec3& dims) : position(pos), size(dims) {}

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

        bool isBoxInFrustum(const FAABB& box) const noexcept
        {
            bool result = true;
            for (auto& plane : m_planes) {
                if (plane.distanceToPoint(box.getVP(plane.normal)) < 0.f)
                    return false;

                else if (plane.distanceToPoint(box.getVN(plane.normal)) < 0.f)
                    result = true;

            }
            return result;
        }

    private:
        struct Plane {
            Plane() = default;

            Plane(const glm::vec3& p1, const glm::vec3& norm) : normal(norm), distance(glm::dot(normal, p1)) {}

            float distanceToPoint(const glm::vec3& point) const { return glm::dot(point, normal) + distance; }

            glm::vec3 normal;
            float distance;
        } m_planes[6];
    };
}