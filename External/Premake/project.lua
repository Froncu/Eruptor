systemversion "latest"
flags { "FatalWarnings", "MultiProcessorCompile" }

absolute_output_directory = "%{cfg.buildcfg}_%{cfg.platform}"
targetdir ("%{prj.location}/../output/" .. absolute_output_directory)
objdir ("%{prj.location}/../intermediate/" .. absolute_output_directory)

filter "configurations:debug"
	symbols "full"
	optimize "off"
	runtime "debug"

filter "configurations:release"
	symbols "full"
	optimize "speed"
	runtime "release"
	defines "NDEBUG"

filter "configurations:distribute"
	symbols "off"
	optimize "speed"
	runtime "release"
	defines "NDEBUG"

filter {}

defines "WIN32"