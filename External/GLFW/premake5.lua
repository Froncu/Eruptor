libdirs "lib-vc2022"
links "glfw3dll"
includedirs "include"
postbuildcommands ("xcopy %[" .. os.getcwd() .. "/lib-vc2022/*.dll] %[%{cfg.buildtarget.directory}] /S /Y")