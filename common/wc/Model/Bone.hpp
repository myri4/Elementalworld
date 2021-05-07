#ifndef BONE_HPP
#define BONE_HPP

#include <vector>
#include <assimp/scene.h>
#include <Maths/AssimpGLMHelpers.hpp>

namespace wc {

struct KeyPosition
{
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation
{
	glm::quat orientation;
	float timeStamp;
};

struct KeyScale
{
	glm::vec3 scale;
	float timeStamp;
};

class Bone {
public:
	Bone(const std::string& name, const int& ID, const aiNodeAnim* channel)
		:
		m_Name(name),
		m_ID(ID),
		m_LocalTransform(1.f)
	{
		m_NumPositions = channel->mNumPositionKeys;

		for (uint32_t positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
		{
			aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
			float timeStamp = (float)channel->mPositionKeys[positionIndex].mTime;
			KeyPosition data;
			data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
			data.timeStamp = timeStamp;
			m_Positions.push_back(data);
		}

		m_NumRotations = channel->mNumRotationKeys;
		for (uint32_t rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
		{
			aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
			float timeStamp = (float)channel->mRotationKeys[rotationIndex].mTime;
			KeyRotation data;
			data.orientation = wc::AssimpGLMHelpers::GetGLMQuat(aiOrientation);
			data.timeStamp = timeStamp;
			m_Rotations.push_back(data);
		}

		m_NumScalings = channel->mNumScalingKeys;
		for (uint32_t keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
		{
			aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
			float timeStamp = (float)channel->mScalingKeys[keyIndex].mTime;
			KeyScale data;
			data.scale = wc::AssimpGLMHelpers::GetGLMVec(scale);
			data.timeStamp = timeStamp;
			m_Scales.push_back(data);
		}
	}

	void Update(const float& animationTime)
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.f), InterpolatePosition(animationTime));
		glm::mat4 rotation = glm::mat4(InterpolateRotation(animationTime));
		glm::mat4 scale = glm::scale(glm::mat4(1.f), InterpolateScaling(animationTime));
		m_LocalTransform = translation * rotation * scale;
	}

	glm::mat4 GetLocalTransform() { return m_LocalTransform; }

	std::string GetBoneName() const { return m_Name; }

	int GetBoneID() { return m_ID; }

	uint32_t GetPositionIndex(const float& animationTime)
	{
		for (uint32_t index = 0; index < m_NumPositions - 1; index++)
		{
			if (animationTime < m_Positions[index + 1].timeStamp)
				return index;
		}
		assert(0);
	}

	uint32_t GetRotationIndex(const float& animationTime)
	{
		for (uint32_t index = 0; index < m_NumRotations - 1; index++)
		{
			if (animationTime < m_Rotations[index + 1].timeStamp)
				return index;
		}
		assert(0);
	}

	uint32_t GetScaleIndex(const float& animationTime)
	{
		for (uint32_t index = 0; index < m_NumScalings - 1; index++)
		{
			if (animationTime < m_Scales[index + 1].timeStamp)
				return index;
		}
		assert(0);
	}

private:

	float GetScaleFactor(const float& lastTimeStamp, const float& nextTimeStamp, const float& animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	glm::vec3 InterpolatePosition(const float& animationTime)
	{
		if (m_NumPositions == 1)
			return m_Positions[0].position;

		uint32_t p0Index = GetPositionIndex(animationTime);
		uint32_t p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
		glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,  m_Positions[p1Index].position, scaleFactor);
		return finalPosition;
	}

	glm::quat InterpolateRotation(const float& animationTime)
	{
		if (m_NumRotations == 1)
		{
			auto rotation = glm::normalize(m_Rotations[0].orientation);
			return rotation; // glm::toMat4(rotation)
		}

		uint32_t p0Index = GetRotationIndex(animationTime);
		uint32_t p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
		glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return finalRotation; // glm::toMat4(finalRotation)
	}

	glm::vec3 InterpolateScaling(const float& animationTime)
	{
		if (m_NumScalings == 1)
			return m_Scales[0].scale;

		uint32_t p0Index = GetScaleIndex(animationTime);
		uint32_t p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,	m_Scales[p1Index].timeStamp, animationTime);
		glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
		return finalScale;
	}

	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale> m_Scales;
	uint32_t m_NumPositions;
	uint32_t m_NumRotations;
	uint32_t m_NumScalings;

	glm::mat4 m_LocalTransform;
	std::string m_Name;
	int m_ID;
};
}

#endif