#ifndef SCENE_HPP
#define SCENE_HPP

#include "sub_mesh.hpp"
#include "renderer/buffer.hpp"
#include "renderer/device.hpp"
#include "renderer/image_view.hpp"

namespace eru
{
   class Scene final
   {
      public:
         Scene(Device const& device, std::filesystem::path const& path);
         Scene(Scene const&) = delete;
         Scene(Scene&&) = default;

         ~Scene() = default;

         Scene& operator=(Scene const&) = delete;
         Scene& operator=(Scene&&) = default;

         [[nodiscard]] Buffer const& vertex_buffer() const;
         [[nodiscard]] Buffer const& index_buffer() const;
         [[nodiscard]] std::span<SubMesh const> sub_meshes() const;
         [[nodiscard]] std::span<std::pair<Image, ImageView> const> diffuse_images() const;

      private:
         void load_submeshes(Device const& device, aiScene const& scene);
         void load_textures(Device const& device, aiMaterial const& material, aiTextureType type,
            std::filesystem::path const& scene_path, BufferBuilder& staging_buffer_builder, ImageBuilder& image_builder,
            ImageViewBuilder& image_view_builder);
         void load_materials(Device const& device, aiScene const& scene, std::filesystem::path const& path);

         Buffer vertex_buffer_{};
         Buffer index_buffer_{};
         std::vector<SubMesh> sub_meshes_{};
         std::vector<std::pair<Image, ImageView>> diffuse_images_{};
         std::vector<Buffer> materials_{};
   };
}

#endif