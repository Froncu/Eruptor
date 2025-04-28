#ifndef MESH_HPP
#define MESH_HPP

#include "erupch/erupch.hpp"
#include "vertex.hpp"

namespace eru
{
   struct Mesh final
   {
      std::vector<Vertex> vertices{};
      std::vector<std::uint32_t> indices{};
   };
}

#endif