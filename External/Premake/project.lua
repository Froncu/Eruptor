language "C++"
cppdialect "C++20"
warnings "high"
systemversion "latest"
flags { "FatalWarnings", "MultiProcessorCompile" }
defines "WIN32"

absolute_output_directory = "%{cfg.buildcfg} (%{cfg.platform})"
targetdir ("%{prj.location}/../Output/" .. absolute_output_directory)
objdir ("%{prj.location}/../Intermediate/" .. absolute_output_directory)

filter "configurations:Debug"
	symbols "full"
	optimize "off"
	runtime "debug"

filter "configurations:Release"
	symbols "full"
	optimize "speed"
	runtime "release"
	defines "NDEBUG"

filter "configurations:Distribute"
	symbols "off"
	optimize "speed"
	runtime "release"
	defines "NDEBUG"

filter {}

filter "platforms:x64"
	defines "PLATFORM_X64"

filter "platforms:x86"
	defines "PLATFORM_X86"

filter {}