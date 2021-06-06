#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP

#include "Animation.hpp"

namespace wc {
class Animator {
public:

	Animator() {}

	void UpdateAnimation(const float& dt){
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = glm::mod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform) // @TODO: Calculate the matricies in the shader
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
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	auto& GetPoseTransforms() const { return m_Transforms; }
private:
	glm::mat4 m_Transforms[MAX_BONE_WEIGHTS] = { glm::mat4(1.0f) };
	Animation* m_CurrentAnimation = nullptr;
	float m_CurrentTime = 0.f;
};
}
#endif