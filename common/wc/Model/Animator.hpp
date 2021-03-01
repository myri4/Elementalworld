#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP

#include "Animation.hpp"

namespace wc {
class Animator {
public:

	Animator()
	{
		for (uint32_t i = 0; i < MAX_BONE_WEIGHTS; i++)
			m_Transforms[i] = glm::mat4(1.0f); // @TODO: Try with memset
	}

	void UpdateAnimation(const float& dt, const gl::Shader& shader)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = glm::mod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), shader);
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, const gl::Shader& shader) // @TODO: CAlculate the matricies in the shader
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())		
			m_Transforms[boneInfoMap[nodeName].id] = globalTransformation * boneInfoMap[nodeName].offset;
		

		for (uint32_t i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation, shader);
	}

	auto GetPoseTransforms()
	{
		return m_Transforms;
	}

private:
	std::array<glm::mat4, MAX_BONE_WEIGHTS> m_Transforms;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;
};
}

#endif