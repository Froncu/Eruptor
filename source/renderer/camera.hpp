#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class Camera final
   {
      public:
         struct Data final
         {
            glm::mat4 view;
            glm::mat4 projection;
         };

         Camera() = default;
         Camera(Camera const&) = delete;
         Camera(Camera&&) = delete;

         ~Camera() = default;

         Camera& operator=(Camera const&) = delete;
         Camera& operator=(Camera&&) = delete;

         void translate_in_direction(glm::vec3 const& translation);
         void translate(glm::vec3 const& translation);
         void rotate(float yaw, float pitch);

         void change_projection_extent(vk::Extent2D extent);
         void change_field_of_view(float field_of_view);
         void change_near_plane(float near_plane);
         void change_far_plane(float far_plane);

         [[nodiscard]] Data const& data() const;

      private:
         void calculate_direction() const;
         void calculate_view_matrix() const;
         void calculate_projection_matrix() const;

         glm::vec3 position_{};
         float yaw_{};
         float pitch_{};

         vk::Extent2D projection_extent_{};
         float field_of_view_{ 45.0f };
         float near_plane_{ 0.01f };
         float far_plane_{ 50.0f };

         mutable glm::vec3 front_{};
         mutable glm::vec3 right_{};
         mutable glm::vec3 up_{};
         mutable Data data_{
            .view{},
            .projection{}
         };

         mutable bool is_direction_dirty_{ true };
         mutable bool is_view_dirty_{ true };
         mutable bool is_projection_dirty_{ true };
   };
}

#endif