function clean_directories(root_directory)
	for index, full_directory_path in ipairs(os.matchdirs(root_directory .. "/*")) do
		local directory_name = path.getbasename(full_directory_path)

		if directory_name == ".vs" or directory_name == "Output" or directory_name == "Intermediate" then
			print("Deleting \"" .. full_directory_path .. "\" ...")
			os.rmdir(full_directory_path)
		else
			clean_directories(full_directory_path)
		end
	end
end

newaction
{
	trigger = "clean",
	description = "clean the generated project and output files",
	execute = function ()
		clean_directories(os.getcwd())
		
		os.remove "**.sln"
		os.remove "**.vcxproj"
		os.remove "**.filters"
		os.remove "**.user"

		print("Directories cleaned!\n")
	end
}

workspace "Eruptor"
	configurations { "Debug",  "Release", "Distribute" }
	platforms "x64"
	architecture "x64"

	include "Eruptor"