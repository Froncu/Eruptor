#ifndef UNIFORM_BUFFER_OBJECT_HPP
#define UNIFORM_BUFFER_OBJECT_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   struct UniformBufferObject final
   {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 projection;
   };
}

#endif