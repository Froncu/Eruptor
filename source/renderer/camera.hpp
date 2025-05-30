#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   struct Camera final
   {
      glm::mat4 view;
      glm::mat4 projection;
   };
}

#endif