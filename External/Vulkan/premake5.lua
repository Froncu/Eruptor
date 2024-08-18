filter "platforms:x64"
	libdirs "$(VULKAN_SDK)/Lib"

filter "platforms:x86"
	libdirs "$(VULKAN_SDK)/Lib32"

filter {}

links "vulkan-1.lib"
includedirs "$(VULKAN_SDK)/Include"