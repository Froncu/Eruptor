#include "scene.hpp"

namespace eru
{
   Scene::Scene(std::filesystem::path const& path)
   {
      Assimp::Importer importer{};
      aiScene const* const scene{
         importer.ReadFile(path.string().c_str(),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_GenNormals)
      };

      if (not scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or not scene->mRootNode)
         throw std::runtime_error("failed to load model!");

      for (aiMesh const* const native_mesh : std::vector<aiMesh*>{ scene->mMeshes, scene->mMeshes + scene->mNumMeshes })
      {
         if (native_mesh->mPrimitiveTypes not_eq aiPrimitiveType_TRIANGLE)
            throw std::runtime_error("model is not triangulated!");

         auto& [vertices, indices]{ meshes.emplace_back() };

         vertices.reserve(vertices.size() + native_mesh->mNumVertices);

         for (unsigned int index{}; index < native_mesh->mNumVertices; ++index)
            vertices.push_back({
               .position{ native_mesh->mVertices[index].x, native_mesh->mVertices[index].z, native_mesh->mVertices[index].y },
               .color{ 1.0f, 1.0f, 1.0f },
               .uv{ native_mesh->mTextureCoords[0][index].x, native_mesh->mTextureCoords[0][index].y }
            });

         indices.reserve(indices.size() + native_mesh->mNumFaces * 3);

         for (unsigned int index{}; index < native_mesh->mNumFaces; ++index)
         {
            aiFace const face{ native_mesh->mFaces[index] };
            indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
         }
      }
   }
}