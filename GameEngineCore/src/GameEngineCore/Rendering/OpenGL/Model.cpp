#include "Model.hpp"
#include "GameEngineCore/Log.hpp";


namespace GameEngine {
    void Model::draw(std::shared_ptr<ShaderProgram> shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].draw(shader);
    }

    Model::Model(
        std::vector<GLfloat> vertices,
        std::vector<unsigned int> indices,
        std::vector<Texture> textures,
        bool hasNormals
    ) {
        if (textures.size()) {
            if (hasNormals) {
                meshes.push_back(
                    Mesh(
                        BufferLayout{
                           ShaderDataType::Float3,
                           ShaderDataType::Float3,
                           ShaderDataType::Float2,
                        },
                        vertices,
                        indices,
                        textures
                        )
                );
            }
            else {
                meshes.push_back(
                    Mesh(
                        BufferLayout{
                            ShaderDataType::Float3,
                            ShaderDataType::Float2,
                        },
                        vertices,
                        indices,
                        textures
                        )
                );
            }
        } 
        else 
        {
            if (hasNormals) {
                meshes.push_back(
                    Mesh(
                        BufferLayout{
                           ShaderDataType::Float3,
                           ShaderDataType::Float3,
                        },
                        vertices,
                        indices,
                        textures
                     )
                );
            }
            else {
                meshes.push_back(
                    Mesh(
                        BufferLayout{
                            ShaderDataType::Float3,
                        },
                        vertices,
                        indices,
                        textures
                    )
                );
            }
        }        
    }

    void Model::loadModel(std::string path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_CRITICAL("ERROR::ASSIMP::{0}", importer.GetErrorString());
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }


    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        // ���������� ��� ������������� ����� � ����(���� ����)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // ��������� �� �� ��������� � ��� ������� ������� ����
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }


    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<GLfloat> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            if (mesh->HasNormals()) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            } 
            if (mesh->mTextureCoords[0]) { 
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
                
        }

        if (mesh->mMaterialIndex >= 0)
        {
     
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                aiTextureType_DIFFUSE, Texture::Type::Diffusal);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<Texture> specularMaps = loadMaterialTextures(material,
                aiTextureType_SPECULAR, Texture::Type::Specular);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(
            BufferLayout{ 
                ShaderDataType::Float3,
                ShaderDataType::Float3,
                ShaderDataType::Float2
            },
            vertices, 
            indices,
            textures
        );
    }

    std::vector<Texture> Model::loadMaterialTextures(aiMaterial * mat, aiTextureType type, Texture::Type typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].getLocation().c_str(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {   // ���� �������� �� ���� ��������� � ������� ���
                
                Texture texture(
                    std::string(directory + '/' + str.C_Str()).c_str(),
                    typeName, 
                    Texture::WrappingMode::Repeat,
                    Texture::MipmapFilterMode::LinearLinear
                );
       
                textures.push_back(texture);
                // ������� �������� � ������ ��� �����������
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
}