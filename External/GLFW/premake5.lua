local relativepath = os.getcwd()

libdirs "lib-vc2022"
links "glfw3dll"
includedirs "include"
postbuildcommands ("xcopy %[" .. relativepath .. "/lib-vc2022/*.dll] %[%{cfg.buildtarget.directory}] /S /Y")