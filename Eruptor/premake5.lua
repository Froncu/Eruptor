project "Eruptor"
	include "../External/Premake/project"
	language "C++"
	cppdialect "C++20"
	warnings "high"
	kind "ConsoleApp"
	files "Source/**.cpp"

	dofile "../External/Vulkan/premake5.lua"
	dofile "../External/GLFW/premake5.lua"
	dofile "../External/GLM/premake5.lua"

project "*"