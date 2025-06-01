#include "builders/buffer_builder.hpp"
#include "vertex.hpp"
#include "scene.hpp"

namespace eru
{
   Scene::Scene(Device const& device, std::filesystem::path const& path)
   {
      Assimp::Importer importer{};
      aiScene const* const scene{
         importer.ReadFile(path.string().c_str(),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_GenNormals)
      };

      if (not scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or not scene->mRootNode)
         throw std::runtime_error("failed to load model!");

      std::vector<Vertex> vertices{};
      std::vector<std::uint32_t> indices{};
      for (aiMesh const* const native_mesh : std::vector<aiMesh*>{ scene->mMeshes, scene->mMeshes + scene->mNumMeshes })
      {
         if (native_mesh->mPrimitiveTypes not_eq aiPrimitiveType_TRIANGLE)
            throw std::runtime_error("model is not triangulated!");

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

      vk::DeviceSize buffer_size{ sizeof(decltype(vertices)::value_type) * vertices.size() };

      auto staging_buffer_builder{
         BufferBuilder{}
         .change_size(buffer_size)
         .change_usage(vk::BufferUsageFlagBits::eTransferSrc)
         .change_sharing_mode(vk::SharingMode::eExclusive)
         .change_allocation_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT)
         .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
         .change_allocation_preferred_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
      };

      auto buffer_builder{
         BufferBuilder{}
         .change_size(buffer_size)
         .change_usage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer)
         .change_sharing_mode(vk::SharingMode::eExclusive)
         .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
      };

      Buffer staging_buffer{ staging_buffer_builder.build(device) };
      staging_buffer.upload(vertices.data(), buffer_size);
      vertex_buffer_ = buffer_builder.build(device);
      staging_buffer.copy(device, vertex_buffer_, buffer_size);

      buffer_size = sizeof(decltype(indices)::value_type) * indices.size();
      staging_buffer = staging_buffer_builder
         .change_size(buffer_size)
         .build(device);
      staging_buffer.upload(indices.data(), buffer_size);
      index_buffer_ = buffer_builder
         .change_size(buffer_size)
         .change_usage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer)
         .build(device);
      staging_buffer.copy(device, index_buffer_, buffer_size);

      index_count_ = static_cast<std::uint32_t>(indices.size());
   }

   Buffer const& Scene::vertex_buffer() const
   {
      return vertex_buffer_;
   }

   Buffer const& Scene::index_buffer() const
   {
      return index_buffer_;
   }

   std::uint32_t Scene::index_count() const
   {
      return index_count_;
   }
}