#include "camera.hpp"

namespace eru
{
   void Camera::translate_in_direction(glm::vec3 const& translation)
   {
      if (not translation.x and not translation.y and not translation.z)
         return;

      if (is_direction_dirty_)
         calculate_direction();

      position_ += translation.x * right_ + translation.y * up_ + translation.z * front_;
      is_view_dirty_ = true;
   }

   void Camera::translate(glm::vec3 const& translation)
   {
      if (not translation.x and not translation.y and not translation.z)
         return;

      position_ += translation;
      is_view_dirty_ = true;
   }

   void Camera::rotate(float const yaw, float const pitch)
   {
      if (not yaw and not pitch)
         return;

      yaw_ += yaw;
      pitch_ = std::clamp(pitch_ + pitch, -89.0f, 89.0f);

      is_direction_dirty_ = true;
   }

   void Camera::change_projection_extent(vk::Extent2D const extent)
   {
      projection_extent_ = extent;
      is_projection_dirty_ = true;
   }

   void Camera::change_field_of_view(float field_of_view)
   {
      field_of_view_ = field_of_view;
      is_projection_dirty_ = true;
   }

   void Camera::change_near_plane(float const near_plane)
   {
      near_plane_ = near_plane;
      is_projection_dirty_ = true;
   }

   void Camera::change_far_plane(float const far_plane)
   {
      far_plane_ = far_plane;
      is_projection_dirty_ = true;
   }

   Camera::Data const& Camera::data() const
   {
      if (is_direction_dirty_)
         calculate_direction();

      if (is_view_dirty_)
         calculate_view_matrix();

      if (is_projection_dirty_)
         calculate_projection_matrix();

      return data_;
   }

   void Camera::calculate_direction() const
   {
      front_.x = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
      front_.y = sin(glm::radians(pitch_));
      front_.z = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));

      front_ = glm::normalize(front_);
      right_ = glm::normalize(glm::cross(front_, glm::vec3(0.0f, -1.0f, 0.0f)));
      up_ = glm::normalize(glm::cross(right_, front_));

      is_direction_dirty_ = false;
      is_view_dirty_ = true;
   }

   void Camera::calculate_view_matrix() const
   {
      data_.view = glm::lookAt(position_, position_ + front_, up_);
      is_view_dirty_ = false;
   }

   void Camera::calculate_projection_matrix() const
   {
      data_.projection = glm::perspective(glm::radians(field_of_view_),
         projection_extent_.width / static_cast<float>(projection_extent_.height),
         near_plane_, far_plane_);
      is_projection_dirty_ = false;
   }
}