#pragma once

#include <assimp/Quaternion.h>
#include <assimp/vector3.h>
#include <assimp/color4.h>
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/quaternion.hpp>

namespace wc {
    namespace AssimpGLMHelpers{

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& m)
    {
        return glm::transpose(glm::make_mat4(&m.a1));;
    }

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix3x3& m)
    {
        return glm::transpose(glm::make_mat3(&m.a1));;
    }

    static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
    {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    static inline glm::vec4 GetGLMVec(const aiColor4D& vec)
    {
        return glm::vec4(vec.r, vec.g, vec.b, vec.a);
    }

    static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }
};
}