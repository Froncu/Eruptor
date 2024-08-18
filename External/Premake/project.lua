systemversion "latest"
flags { "FatalWarnings", "MultiProcessorCompile" }

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

defines "WIN32"