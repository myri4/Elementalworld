#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "Bone.hpp"
#include <functional>
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
	Animation() = default;

	Animation(const std::string& animationPath, Model* model) {	Create(animationPath, model); }

	void Create(const std::string& animationPath, Model* model) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[0];
		m_Duration = (float)animation->mDuration;
		m_TicksPerSecond = (float)animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		SetupBones(animation, *model);
	}

	~Animation() = default;

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


	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration; }
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
	inline const auto& GetBoneIDMap() {	return m_BoneInfoMap; }

private:
	void SetupBones(const aiAnimation* animation, Model& model)	{
		uint32_t size = animation->mNumChannels;

		auto& boneInfoMap = model.GetOffsetMatMap();
		int& boneCount = model.GetBoneCount();

		for (uint32_t i = 0; i < size; i++)	{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = wc::AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (uint32_t i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
	float m_Duration = 0.f;
	float m_TicksPerSecond = 0.f;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
};
}

#endif