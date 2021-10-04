#pragma once

#include "Bone.hpp"
#include "Model.hpp"

namespace wc {

struct AssimpNodeData
{
	glm::mat4 transformation = glm::mat4(1.f);
	std::string name;
	uint32_t childrenCount = 0;
	std::vector<AssimpNodeData> children;
};

class Animation {
public:
	Animation() { memset(m_Transforms, 1, sizeof(m_Transforms)); };

	//Animation(const std::string& animationPath, Model& model) {	Create(animationPath, model); }

	void Create(const std::string& animationPath, Model& model) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		auto& animation = scene->mAnimations[0];
		m_Duration = (float)animation->mDuration;
		m_TicksPerSecond = (float)animation->mTicksPerSecond;
		ReadHeirarchyData(m_RootNode, scene->mRootNode);		
		//SetupBones

		auto& offsetMatMap = model.m_OffsetMatMap;
		uint32_t boneCount = model.m_OffsetMatMap.size();
		m_Bones.reserve(animation->mNumChannels);
		for (uint32_t i = 0; i < animation->mNumChannels; i++) {
			auto aiChannel = animation->mChannels[i];
			const char* boneName = aiChannel->mNodeName.data;

			if (offsetMatMap.find(boneName) == offsetMatMap.end())
			{
				offsetMatMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.emplace_back(aiChannel->mNodeName.data, aiChannel);
		}

		m_BoneInfoMap = &offsetMatMap;
	}

	~Animation() = default;	

	void Play(const float& startTime = 0.f) { m_CurrentTime = startTime; }

	void Update(const float& dt) {		
		m_CurrentTime += m_TicksPerSecond * dt;
		m_CurrentTime = glm::mod(m_CurrentTime, m_Duration);
		CalculateBoneTransform(m_RootNode, glm::mat4(1.0f));
	}

	auto& GetPoseTransforms() { return m_Transforms; }

private:
	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	void CalculateBoneTransform(const AssimpNodeData& node, const glm::mat4& parentTransform) // @TODO: Calculate the matricies in the shader
	{
		const std::string& nodeName = node.name;
		glm::mat4& nodeTransform = (glm::mat4&)node.transformation;

		Bone* Bone = FindBone(nodeName);

		if (Bone) nodeTransform = Bone->Update(m_CurrentTime);		

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		if (m_BoneInfoMap->find(nodeName) != m_BoneInfoMap->end())
			m_Transforms[m_BoneInfoMap->at(nodeName).id] = globalTransformation * m_BoneInfoMap->at(nodeName).offset;

		for (uint32_t i = 0; i < node.childrenCount; i++)
			CalculateBoneTransform(node.children[i], globalTransformation);
	}


	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = wc::AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;
		dest.children.reserve(src->mNumChildren);
		for (uint32_t i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.emplace_back(newData);
		}
	}
	float m_Duration = 0.f;
	float m_TicksPerSecond = 0.f;
	float m_CurrentTime = 0.f;
	std::vector<Bone> m_Bones;
	std::unordered_map<std::string, BoneInfo>* m_BoneInfoMap;
	AssimpNodeData m_RootNode;
	glm::mat4 m_Transforms[MAX_BONE_WEIGHTS] = { glm::mat4(1.f) };
};
}