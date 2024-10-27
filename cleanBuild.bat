DEL /S /Q *.sln, *.vcxproj, *.filters, *.user
RMDIR /S /Q .vs
RMDIR /S /Q output
RMDIR /S /Q intermediate

External\Premake\premake5.exe vs2022
PAUSE