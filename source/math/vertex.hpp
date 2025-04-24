#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "erupch.hpp"

namespace eru
{
   struct Vertex final
   {
      static std::array<vk::VertexInputBindingDescription, 1> const BINDING_DESCRIPTIONS;
      static std::array<vk::VertexInputAttributeDescription, 2> const ATTRIBUTE_DESCRIPTIONS;

      glm::vec2 position;
      glm::vec3 color;
   };
}

#endif