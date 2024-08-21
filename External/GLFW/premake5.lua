local relativepath = os.getcwd()

libdirs "%{cfg.platform}/lib-vc2022"
links "glfw3dll"
includedirs "%{cfg.platform}/include"
postbuildcommands ("xcopy %[" .. relativepath .. "/%{cfg.platform}/lib-vc2022/*.dll] %[%{cfg.buildtarget.directory}] /S /Y")