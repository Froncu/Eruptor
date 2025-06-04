#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace eru
{
   struct Material final
   {
      std::int32_t base_color_index{ -1 };
      std::int32_t normal_index{ -1 };
      std::int32_t metalness_index{ -1 };
   };
}

#endif