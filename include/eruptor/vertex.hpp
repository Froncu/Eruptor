#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "pch.hpp"

namespace eru
{
   struct Vertex final
   {
      static std::array<vk::VertexInputBindingDescription, 1> const INPUT_BINDING_DESCRIPTIONS;
      static std::array<vk::VertexInputAttributeDescription, 3> const INPUT_ATTRIBUTE_DESCRIPTIONS;

      glm::vec2 position;
      glm::vec3 color;
      glm::vec2 texture_coordinate;
   };
}

#endif