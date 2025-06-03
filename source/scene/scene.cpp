#include "scene.hpp"
#include "vertex.hpp"
#include "builders/buffer_builder.hpp"
#include "builders/image_builder.hpp"
#include "builders/image_view_builder.hpp"
#include "utility/exception.hpp"

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

      for (std::uint32_t index{}; index < scene->mNumMaterials; ++index)
      {
         aiMaterial const* const material{ scene->mMaterials[index] };

         aiString diffuse_relative_path{};
         material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_relative_path);
         std::filesystem::path const diffuse_path{ path.parent_path() /= diffuse_relative_path.C_Str() };
         if (not std::filesystem::is_regular_file(diffuse_path))
            continue;

         UniquePointer<SDL_Surface> diffuse_texture{ IMG_Load(diffuse_path.string().c_str()), SDL_DestroySurface };
         diffuse_texture.reset(SDL_ConvertSurface(diffuse_texture.get(), SDL_PIXELFORMAT_RGBA32));

         auto const image_size{ static_cast<vk::DeviceSize>(diffuse_texture->pitch * diffuse_texture->h) };
         vk::Extent3D const image_extent{
            .width{ static_cast<std::uint32_t>(diffuse_texture->w) },
            .height{ static_cast<std::uint32_t>(diffuse_texture->h) },
            .depth{ 1 }
         };

         Buffer staging_buffer{
            BufferBuilder{}
            .change_size(image_size)
            .change_usage(vk::BufferUsageFlagBits::eTransferSrc)
            .change_sharing_mode(vk::SharingMode::eExclusive)
            .change_allocation_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .change_allocation_required_flags(
               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
            .change_allocation_preferred_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
            .build(device)
         };

         staging_buffer.upload(diffuse_texture->pixels, static_cast<std::size_t>(image_size));

         Image diffuse_image{
            ImageBuilder{}
            .change_type(vk::ImageType::e2D)
            .change_format(vk::Format::eR8G8B8A8Srgb)
            .change_extent(image_extent)
            .change_mip_levels(1)
            .change_array_layers(1)
            .change_samples(vk::SampleCountFlagBits::e1)
            .change_tiling(vk::ImageTiling::eOptimal)
            .change_usage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
            .change_sharing_mode(vk::SharingMode::eExclusive)
            .change_initial_layout(vk::ImageLayout::eUndefined)
            .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
            .build(device)
         };

         diffuse_image.transition_layout(device, vk::ImageLayout::eTransferDstOptimal);
         staging_buffer.copy(device, diffuse_image, image_extent);
         diffuse_image.transition_layout(device, vk::ImageLayout::eShaderReadOnlyOptimal);

         ImageView diffuse_image_view{
            ImageViewBuilder{}
            .change_view_type(vk::ImageViewType::e2D)
            .change_format(diffuse_image.info().format)
            .change_subresource_range({
               .aspectMask{ vk::ImageAspectFlagBits::eColor },
               .levelCount{ 1 },
               .layerCount{ 1 }
            })
            .build(device, diffuse_image)
         };

         diffuse_images_.emplace(index,
            std::pair{ std::move(diffuse_image), std::move(diffuse_image_view) });
      }

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
      staging_buffer.upload(vertices.data(), static_cast<std::size_t>(buffer_size));
      vertex_buffer_ = buffer_builder.build(device);
      staging_buffer.copy(device, vertex_buffer_, buffer_size);

      buffer_size = sizeof(decltype(indices)::value_type) * indices.size();
      staging_buffer =
         staging_buffer_builder
         .change_size(buffer_size)
         .build(device);
      staging_buffer.upload(indices.data(), static_cast<std::size_t>(buffer_size));
      index_buffer_ =
         buffer_builder
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

   std::unordered_map<std::uint32_t, std::pair<Image, ImageView>> const& Scene::diffuse_images() const
   {
      return diffuse_images_;
   }
}