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
         [[nodiscard]] std::span<std::pair<Image, ImageView> const> base_color_images() const;
         [[nodiscard]] std::span<std::pair<Image, ImageView> const> normal_images() const;
         [[nodiscard]] std::span<std::pair<Image, ImageView> const> metalness_images() const;
         [[nodiscard]] Buffer const& materials() const;
         [[nodiscard]] std::size_t materials_count() const;

      private:
         void load_submeshes(Device const& device, aiScene const& scene);
         [[nodiscard]] int load_texture(Device const& device, aiMaterial const& material, aiTextureType type,
            std::filesystem::path const& scene_path, BufferBuilder& staging_buffer_builder, ImageBuilder& image_builder,
            ImageViewBuilder& image_view_builder);
         void load_materials(Device const& device, aiScene const& scene, std::filesystem::path const& path);

         Buffer vertex_buffer_{};
         Buffer index_buffer_{};
         std::vector<SubMesh> sub_meshes_{};
         std::vector<std::pair<Image, ImageView>> base_color_images_{};
         std::vector<std::pair<Image, ImageView>> normal_images_{};
         std::vector<std::pair<Image, ImageView>> metalness_images_{};
         Buffer materials_{};
         std::size_t materials_count_{};
   };
}

#endif