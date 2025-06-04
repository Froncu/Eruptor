#include "vertex.hpp"

namespace eru
{
   std::array<vk::VertexInputBindingDescription, 1> const Vertex::BINDING_DESCRIPTIONS{
      {
         {
            .binding{ 0 },
            .stride{ static_cast<std::uint32_t>(sizeof(Vertex)) },
            .inputRate{ vk::VertexInputRate::eVertex }
         }
      }
   };

   std::array<vk::VertexInputAttributeDescription, 2> const Vertex::ATTRIBUTE_DESCRIPTIONS{
      {
         {
            .location{ 0 },
            .binding{ BINDING_DESCRIPTIONS[0].binding },
            .format{ vk::Format::eR32G32B32Sfloat },
            .offset{ offsetof(Vertex, position) }
         },
         {
            .location{ 1 },
            .binding{ BINDING_DESCRIPTIONS[0].binding },
            .format{ vk::Format::eR32G32Sfloat },
            .offset{ offsetof(Vertex, uv) }
         }
      }
   };
}