#ifndef SCENE_HPP
#define SCENE_HPP

#include "mesh/mesh.hpp"

namespace eru
{
   class Scene final
   {
      public:
         explicit Scene(std::filesystem::path const& path);
         Scene(Scene const&) = default;
         Scene(Scene&&) = default;

         ~Scene() = default;

         Scene& operator=(Scene const&) = default;
         Scene& operator=(Scene&&) = default;

         std::vector<Mesh> meshes{};
   };
}

#endif