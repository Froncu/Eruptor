local relativepath = os.getcwd()

libdirs "%{cfg.platform}/lib-vc2022"
links "glfw3"
includedirs "%{cfg.platform}/include"