#ifndef SHADER_STRUCTURE_HPP
#define SHADER_STRUCTURE_HPP

#include "eruptor/pch.hpp"

#define ERU_SHADER_STRUCTURE(structure_name)\
   struct alignas(4) structure_name final

#define ERU_SHADER_FIELD(type, field_name)\
   alignas(4) type field_name

#define ERU_SHADER_ARRAY_FIELD(type, count, field_name)\
   alignas(4) std::array<type, count> field_name

#define ERU_SHADER_TYPE(type) #type

#endif