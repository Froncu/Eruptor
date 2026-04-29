#include "eruptor/vertex.hpp"

namespace eru
{
   decltype(Vertex::INPUT_BINDING_DESCRIPTIONS) Vertex::INPUT_BINDING_DESCRIPTIONS{
      {
         {
            .binding{ 0 },
            .stride{ static_cast<std::uint32_t>(sizeof(Vertex)) },
            .inputRate{ vk::VertexInputRate::eVertex }
         }
      }
   };

   decltype(Vertex::INPUT_ATTRIBUTE_DESCRIPTIONS) Vertex::INPUT_ATTRIBUTE_DESCRIPTIONS{
      {
         {
            .location{ 0 },
            .binding{ 0 },
            .format{ vk::Format::eR32G32B32Sfloat },
            .offset{ offsetof(Vertex, position) }
         },
         {
            .location{ 1 },
            .binding{ 0 },
            .format{ vk::Format::eR32G32B32Sfloat },
            .offset{ offsetof(Vertex, color) }
         },
         {
            .location{ 2 },
            .binding{ 0 },
            .format{ vk::Format::eR32G32Sfloat },
            .offset{ offsetof(Vertex, texture_coordinate) }
         }
      }
   };
}