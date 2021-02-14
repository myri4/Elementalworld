#ifndef MODEL_HPP
#define MODEL_HPP

#include <gl/Texture.hpp>
#include <gl/Vertex.hpp>
#include <gl/IndexBuffer.hpp>
#include <gl/Shaders.hpp>
#include <Renderer/Renderer.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Maths/AssimpGLMHelpers.hpp>

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS 100

namespace wc {

    struct MeshVertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;

        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];

        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
    };

    struct MeshTexture {
        uint32_t id;
        std::string type;
        std::string path;
    };

    class Mesh {
    public:
        // mesh Data
        std::vector<MeshTexture> textures;
        uint32_t VAO;

        // constructor
        Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<MeshTexture>& textures)
        {
            this->textures = textures;

            // now that we have all the required data, set the vertex buffers and its attribute pointers.
            // create buffers/arrays

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            m_VertexBuffer.Create(vertices.data(), vertices.size() * sizeof(MeshVertex), GL_STATIC_DRAW);
            // load data into vertex buffers
            // A great thing about structs is that their memory layout is sequential for all its items.
            // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
            // again translates to 3/2 floats which translates to a byte array.

            m_IndexBuffer.Create(&indices[0], indices.size() * sizeof(uint32_t), GL_STATIC_DRAW);

            indexSize = indices.size();

            // set the vertex attribute pointers
            // vertex Positions
            Renderer::VertexAttribPointer(0, 3, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Position));
            // vertex normals
            Renderer::VertexAttribPointer(1, 3, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Normal));
            // vertex texture coords
            Renderer::VertexAttribPointer(2, 2, sizeof(MeshVertex), (void*)offsetof(MeshVertex, TexCoords));
            // ids
            Renderer::VertexAttribIntPointer(3, 4, sizeof(MeshVertex), (void*)offsetof(MeshVertex, m_BoneIDs));
            // weights
            Renderer::VertexAttribPointer(4, 4, sizeof(MeshVertex), (void*)offsetof(MeshVertex, m_Weights));

            glBindVertexArray(0);
        }

        // render the mesh
        void Draw(const gl::Shader& shader) const
        {
            // bind appropriate textures
            uint32_t diffuseNr = 1;
            uint32_t specularNr = 1;
            uint32_t normalNr = 1;
            uint32_t heightNr = 1;
            for (uint32_t i = 0; i < textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                // retrieve texture number (the N in diffuse_textureN)
                std::string number;
                std::string name = textures[i].type;
                if (name == "texture_diffuse")
                    number = std::to_string(diffuseNr++);
                else if (name == "texture_specular")
                    number = std::to_string(specularNr++); // transfer uint32_t to stream
                else if (name == "texture_normal")
                    number = std::to_string(normalNr++); // transfer uint32_t to stream
                else if (name == "texture_height")
                    number = std::to_string(heightNr++); // transfer uint32_t to stream

                // now set the sampler to the correct texture unit
                shader.setInt((name + number).c_str(), i);
                // and finally bind the texture
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }

            // draw mesh
            glBindVertexArray(VAO);
            Renderer::DrawIndexed(indexSize);

            // always good practice to set everything back to defaults once configured.
            glActiveTexture(GL_TEXTURE0);
        }

    private:
        uint32_t indexSize = 0;
        // render data 
        gl::IndexBuffer m_IndexBuffer;
        gl::VertexBuffer m_VertexBuffer;
        gl::VertexArray m_VertexArray;
    };

    uint32_t TextureFromFile(const char* path) {
        uint32_t textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        auto* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else        
            WC_ERROR("Texture failed to load at path: {0}", path);
        
            stbi_image_free(data);
        return textureID;
    }

    struct BoneInfo
    {
        /*id is index in finalBoneMatrices*/
        int id;

        /*offset matrix transforms vertex from model space to bone space*/
        glm::mat4 offset;

    };

    class Model
    {
    public:
        // model data 
        std::vector<MeshTexture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
        std::vector<Mesh>    meshes;
        std::string directory;

        // constructor, expects a filepath to a 3D model.
        Model(const char* path)
        {
            Create(path);
        }

        Model() = default;

        void Create(const char* path) {
            loadModel(path);
        }

        // draws the model, and thus all its meshes
        void Draw(const gl::Shader& shader) const
        {
            for (uint32_t i = 0; i < meshes.size(); i++)
                meshes[i].Draw(shader);
        }

        auto& GetOffsetMatMap() { return m_BoneInfoMap; }
        int& GetBoneCount() { return m_BoneCounter; }

    private:

        std::map<std::string, BoneInfo> m_BoneInfoMap; //
        int m_BoneCounter = 0;

        void SetVertexBoneDataToDefault(MeshVertex& vertex)
        {
            for (int i = 0; i < MAX_BONE_WEIGHTS; i++)
            {
                vertex.m_BoneIDs[i] = -1;
                vertex.m_Weights[i] = 0.0f;
            }
        }

        void SetVertexBoneData(MeshVertex& vertex, int boneID, float weight)
        {
            for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
            {
                if (vertex.m_BoneIDs[i] < 0)
                {
                    vertex.m_BoneIDs[i] = boneID;
                    vertex.m_Weights[i] = weight;
                    break;
                }
            }
        }

        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        void loadModel(const std::string& path)
        {
            // read file via ASSIMP
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
            // check for errors
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
            {
                WC_ERROR("ASSIMP: {0}", importer.GetErrorString());
                return;
            }
            // retrieve the directory path of the filepath
            directory = path.substr(0, path.find_last_of('/'));

            // process ASSIMP's root node recursively
            processNode(scene->mRootNode, scene);
        }

        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(aiNode* node, const aiScene* scene)
        {
            // process each mesh located at the current node
            for (uint32_t i = 0; i < node->mNumMeshes; i++)
            {
                // the node object only contains indices to index the actual objects in the scene. 
                // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(processMesh(mesh, scene));
            }
            // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
            for (uint32_t i = 0; i < node->mNumChildren; i++)
            {
                processNode(node->mChildren[i], scene);
            }

        }

        Mesh processMesh(aiMesh* mesh, const aiScene* scene)
        {
            std::vector<MeshVertex> vertices;
            std::vector<uint32_t> indices;
            std::vector<MeshTexture> textures;

            for (uint32_t i = 0; i < mesh->mNumVertices; i++)
            {
                MeshVertex vertex;
                //SetVertexBoneDataToDefault(vertex);
                vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
                vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

                if (mesh->mTextureCoords[0])
                {
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i].x;
                    vec.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = vec;
                }
                else
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }
            for (uint32_t i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (uint32_t j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<MeshTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            std::vector<MeshTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            std::vector<MeshTexture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

            ExtractBoneWeightForVertices(vertices, mesh, scene);

            return Mesh(vertices, indices, textures);
        }   

        void ExtractBoneWeightForVertices(std::vector<MeshVertex>& vertices, aiMesh* mesh, const aiScene* scene)
        {
            auto& boneInfoMap = m_BoneInfoMap;
            int& boneCount = m_BoneCounter;

            for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
            {
                int boneID = -1;
                std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
                if (boneInfoMap.find(boneName) == boneInfoMap.end())
                {
                    BoneInfo newBoneInfo;
                    newBoneInfo.id = boneCount;
                    newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                    boneInfoMap[boneName] = newBoneInfo;
                    boneID = boneCount;
                    boneCount++;
                }
                else
                {
                    boneID = boneInfoMap[boneName].id;
                }
                assert(boneID != -1);
                auto weights = mesh->mBones[boneIndex]->mWeights;
                int numWeights = mesh->mBones[boneIndex]->mNumWeights;

                for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
                {
                    int vertexId = weights[weightIndex].mVertexId;
                    float weight = weights[weightIndex].mWeight;
                    assert(vertexId <= vertices.size());
                    //SetVertexBoneData(vertices[vertexId], boneID, weight);
                }
            }
        }

        // checks all material textures of a given type and loads the textures if they're not loaded yet.
        // the required info is returned as a Texture struct.
        std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
        {
            std::vector<MeshTexture> textures;
            for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);
                // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
                bool skip = false;
                for (uint32_t j = 0; j < textures_loaded.size(); j++)
                {
                    if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                    {
                        textures.push_back(textures_loaded[j]);
                        skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                        break;
                    }
                }
                if (!skip)
                {   // if texture hasn't been loaded already, load it
                    MeshTexture texture;
                    std::string dir = this->directory + '/' + std::string(str.C_Str());
                    texture.id = TextureFromFile(dir.c_str());
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
                }
            }
            return textures;
        }
    };

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

    class Bone
    {
    private:
        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale> m_Scales;
        int m_NumPositions;
        int m_NumRotations;
        int m_NumScalings;

        glm::mat4 m_LocalTransform;
        std::string m_Name;
        int m_ID;
    public:
        /*reads keyframes from aiNodeAnim*/
        Bone(const std::string& name, const int& ID, const aiNodeAnim* channel)
            :
            m_Name(name),
            m_ID(ID),
            m_LocalTransform(1.0f)
        {
            m_NumPositions = channel->mNumPositionKeys;

            for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
            {
                aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
                float timeStamp = channel->mPositionKeys[positionIndex].mTime;
                KeyPosition data;
                data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
                data.timeStamp = timeStamp;
                m_Positions.push_back(data);
            }

            m_NumRotations = channel->mNumRotationKeys;
            for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
            {
                aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                KeyRotation data;
                data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
                data.timeStamp = timeStamp;
                m_Rotations.push_back(data);
            }

            m_NumScalings = channel->mNumScalingKeys;
            for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
            {
                aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                KeyScale data;
                data.scale = AssimpGLMHelpers::GetGLMVec(scale);
                data.timeStamp = timeStamp;
                m_Scales.push_back(data);
            }
        }

        /* Interpolates b/w positions,rotations & scaling keys based on the curren time of the
        animation and prepares the local transformation matrix by combining all keys tranformations */
        void Update(const float& animationTime)
        {
            glm::mat4 translation = InterpolatePosition(animationTime);
            glm::mat4 rotation = InterpolateRotation(animationTime);
            glm::mat4 scale = InterpolateScaling(animationTime);
            m_LocalTransform = translation * rotation * scale;
        }

        glm::mat4 GetLocalTransform() { return m_LocalTransform; }
        std::string GetBoneName() const { return m_Name; }
        int GetBoneID() { return m_ID; }

        /* Gets the current index on mKeyPositions to interpolate to based on the current
        animation time */
        int GetPositionIndex(const float& animationTime)
        {
            for (int index = 0; index < m_NumPositions - 1; ++index)
            {
                if (animationTime < m_Positions[index + 1].timeStamp)
                    return index;
            }
            assert(0);
        }

        /* Gets the current index on mKeyRotations to interpolate to based on the current
        animation time */
        int GetRotationIndex(const float& animationTime)
        {
            for (int index = 0; index < m_NumRotations - 1; ++index)
            {
                if (animationTime < m_Rotations[index + 1].timeStamp)
                    return index;
            }
            assert(0);
        }

        /* Gets the current index on mKeyScalings to interpolate to based on the current
        animation time */
        int GetScaleIndex(const float& animationTime)
        {
            for (int index = 0; index < m_NumScalings - 1; ++index)
            {
                if (animationTime < m_Scales[index + 1].timeStamp)
                    return index;
            }
            assert(0);
        }
    private:

        /* Gets normalized value for Lerp & Slerp*/
        float GetScaleFactor(const float& lastTimeStamp, const float& nextTimeStamp, const float& animationTime)
        {
            float scaleFactor = 0.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        /* figures out which position keys to interpolate b/w and performs the interpolation
        and returns the translation matrix */
        glm::mat4 InterpolatePosition(const float& animationTime)
        {
            if (1 == m_NumPositions)
                return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

            int p0Index = GetPositionIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                m_Positions[p1Index].timeStamp, animationTime);
            glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
                m_Positions[p1Index].position
                , scaleFactor);
            return glm::translate(glm::mat4(1.0f), finalPosition);
        }

        /* figures out which rotations keys to interpolate b/w and performs the interpolation
        and returns the rotation matrix */
        glm::mat4 InterpolateRotation(const float& animationTime)
        {
            if (1 == m_NumRotations)
            {
                auto rotation = glm::normalize(m_Rotations[0].orientation);
                return glm::mat4(rotation);
            }

            int p0Index = GetRotationIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
                m_Rotations[p1Index].timeStamp, animationTime);
            glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
                m_Rotations[p1Index].orientation, scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::mat4(finalRotation);
        }

        /* figures out which scaling keys to interpolate b/w and performs the interpolation
        and returns the scale matrix */
        glm::mat4 InterpolateScaling(const float& animationTime)
        {
            if (1 == m_NumScalings)
                return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

            int p0Index = GetScaleIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
                m_Scales[p1Index].timeStamp, animationTime);
            glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale,
                m_Scales[p1Index].scale, scaleFactor);
            return glm::scale(glm::mat4(1.0f), finalScale);
        }
    };

    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
    public:
        Animation() = default;

        Animation(const std::string& animationPath, Model* model)
        {
            Create(animationPath, model);
        }

        void Create(const std::string& animationPath, Model* model) {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
            assert(scene && scene->mRootNode);
            auto animation = scene->mAnimations[0];
            m_Duration = animation->mDuration;
            m_TicksPerSecond = animation->mTicksPerSecond;
            ReadHeirarchyData(m_RootNode, scene->mRootNode);
            ReadMissingBones(animation, *model);
        }

        ~Animation() {}

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

        inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }

    private:
        void ReadMissingBones(const aiAnimation* animation, Model& model)
        {
            int size = animation->mNumChannels;

            auto& boneInfoMap = model.GetOffsetMatMap();//getting m_BoneInfoMap from Model class
            int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

            //reading channels(bones engaged in an animation and their keyframes)
            for (int i = 0; i < size; i++)
            {
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

        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
        {
            assert(src);

            dest.name = src->mName.data;
            dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
            dest.childrenCount = src->mNumChildren;

            for (int i = 0; i < src->mNumChildren; i++)
            {
                AssimpNodeData newData;
                ReadHeirarchyData(newData, src->mChildren[i]);
                dest.children.push_back(newData);
            }
        }
        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
    };

    class Animator
    {
    public:
        Animator(Animation* Animation)
        {
            m_CurrentTime = 0.0;
            m_CurrentAnimation = m_CurrentAnimation;

            m_FinalBoneMatrices.reserve(MAX_BONE_WEIGHTS);

            for (uint8_t i = 0; i < MAX_BONE_WEIGHTS; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
        }

        Animator() = default;

        void Create(Animation* Animation){
            m_CurrentTime = 0.0;
            m_CurrentAnimation = m_CurrentAnimation;

            m_FinalBoneMatrices.reserve(MAX_BONE_WEIGHTS);

            for (uint8_t i = 0; i < MAX_BONE_WEIGHTS; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
        }

        void UpdateAnimation(const float& dt)
        {
            m_DeltaTime = dt;
            if (m_CurrentAnimation)
            {
                m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
                CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
            }
        }

        void PlayAnimation(Animation* pAnimation)
        {
            m_CurrentAnimation = pAnimation;
            m_CurrentTime = 0.0f;
        }

        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
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
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
                CalculateBoneTransform(&node->children[i], globalTransformation);
        }

        std::vector<glm::mat4> GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }

    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime = 0.f;
        float m_DeltaTime = 0.f;
    };
}
#endif