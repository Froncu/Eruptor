#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   struct Vertex final
   {
      static std::array<vk::VertexInputBindingDescription, 1> const BINDING_DESCRIPTIONS;
      static std::array<vk::VertexInputAttributeDescription, 4> const ATTRIBUTE_DESCRIPTIONS;

      glm::vec3 position;
      glm::vec2 uv;
      glm::vec3 normal;
      glm::vec3 tangent;
   };
}

#endif