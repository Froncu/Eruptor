project "Eruptor"
   include "../external/Premake/project"
   language "C++"
   cppdialect "C++20"
   warnings "high"
   kind "ConsoleApp"
   files { "source/**.cpp", "source/**.c" }
   includedirs "source"
   pchheader "erupch.hpp"
   pchsource "source/erupch.cpp"

   filter "configurations:debug"
      linkoptions { "/ignore:4099" }
   filter {}

   dofile "../external/Vulkan/premake5.lua"
   dofile "../external/GLFW/premake5.lua"
   dofile "../external/GLM/premake5.lua"
   
   postbuildcommands {
      "mkdir %[%{cfg.buildtarget.directory}resources]",
      "xcopy %[resources/.] %[%{cfg.buildtarget.directory}resources] /S /Y"
   }
project "*"