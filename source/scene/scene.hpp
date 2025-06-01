#ifndef SCENE_HPP
#define SCENE_HPP

#include "renderer/buffer.hpp"
#include "renderer/device.hpp"

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
         [[nodiscard]] std::uint32_t index_count() const;

      private:
         Buffer vertex_buffer_{};
         Buffer index_buffer_{};
         std::uint32_t index_count_{};
   };
}

#endif