libdirs "$(VULKAN_SDK)/Lib"
links "vulkan-1.lib"

filter "configurations:debug"
   links "shaderc_combinedd.lib"
filter "configurations:distribute or configurations:release"
   links "shaderc_combined.lib"
filter {}

includedirs "$(VULKAN_SDK)/Include"