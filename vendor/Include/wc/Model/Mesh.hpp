#pragma once

#include <gl/Buffer.hpp>
#include <gl/Texture.hpp>
#include <gl/VertexArray.hpp>

#include <vector>

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS MAX_BONE_INFLUENCE * 25

struct MeshVertex {
    // position
    glm::vec3 Position = glm::vec3(0.f);
    // normal
    glm::vec3 Normal = glm::vec3(0.f);
    // texCoords
    glm::vec2 TexCoords = glm::vec2(0.f);

    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE] = { -1 };

    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE] = { 0.f };

    MeshVertex() {
        memset(m_BoneIDs,-1, sizeof(m_BoneIDs));
        memset(m_Weights, 0, sizeof(m_Weights));
    }
};
