#include "builders/buffer_builder.hpp"
#include "scene.hpp"
#include "utility/exception.hpp"
#include "vertex.hpp"

namespace eru
{
   Scene::Scene(Device const& device, std::filesystem::path const& path)
   {
      if (not std::filesystem::exists(path))
         exception("model file \"{}\" does not exist!", path.string());

      Assimp::Importer importer{};
      aiScene const* const scene{
         importer.ReadFile(path.string().c_str(),
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_ConvertToLeftHanded |
            aiProcess_PreTransformVertices |
            aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeMeshes |
            aiProcess_ValidateDataStructure
         )
      };

      if (not scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or not scene->mRootNode)
         exception("failed to load model ({})", aiGetErrorString());

      std::int32_t vertex_offset{};
      std::uint32_t index_offset{};
      std::vector<Vertex> vertices{};
      std::vector<std::uint32_t> indices{};
      for (aiMesh const* const mesh : std::span{ scene->mMeshes, scene->mMeshes + scene->mNumMeshes })
      {
         std::uint32_t const vertex_count{ mesh->mNumVertices };
         vertices.reserve(vertices.size() + vertex_count);
         for (std::uint32_t index{}; index < vertex_count; ++index)
            vertices.push_back({
               .position{ mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z },
               .color{ 1.0f, 1.0f, 1.0f },
               .uv{ mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y }
            });

         std::uint32_t const index_count{ mesh->mNumFaces * 3 };
         indices.reserve(indices.size() + index_count);
         for (aiFace const& face : std::span{ mesh->mFaces, mesh->mFaces + mesh->mNumFaces })
            for (std::uint32_t const index : std::span{ face.mIndices, face.mIndices + face.mNumIndices })
               indices.push_back(index);

         sub_meshes_.push_back({
            .vertex_offset{ vertex_offset },
            .index_offset{ index_offset },
            .index_count{ index_count },
            .material_index{ mesh->mMaterialIndex }
         });

         vertex_offset += vertex_count;
         index_offset += index_count;
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
   }

   Buffer const& Scene::vertex_buffer() const
   {
      return vertex_buffer_;
   }

   Buffer const& Scene::index_buffer() const
   {
      return index_buffer_;
   }

   std::span<SubMesh const> Scene::sub_meshes() const
   {
      return sub_meshes_;
   }
}