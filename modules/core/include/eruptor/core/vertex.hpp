#pragma once

#include <array>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace eru
{
   struct Vertex final
   {
      static std::array<vk::VertexInputBindingDescription, 1> const INPUT_BINDING_DESCRIPTIONS;
      static std::array<vk::VertexInputAttributeDescription, 3> const INPUT_ATTRIBUTE_DESCRIPTIONS;

      glm::vec3 position;
      glm::vec3 color;
      glm::vec2 texture_coordinate;
   };
}