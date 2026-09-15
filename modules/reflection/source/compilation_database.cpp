#include <clang/Options/OptionUtils.h>

#include "compilation_database.hpp"

namespace tep
{
   CompilationDatabase::CompilationDatabase(clang::tooling::CompilationDatabase const& native_database)
      : native_database_{ native_database }
   {
   }

   auto CompilationDatabase::getCompileCommands(llvm::StringRef const file_path) const -> std::vector<clang::tooling::CompileCommand>
   {
      std::vector commands{ native_database_.getCompileCommands(file_path) };
      for (clang::tooling::CompileCommand& command : commands)
         if (not command.CommandLine.empty())
            command.CommandLine.insert(std::next(command.CommandLine.begin()), { "-resource-dir", clang::GetResourcesPath(command.CommandLine.front()) });

      return commands;
   }

   auto CompilationDatabase::getAllFiles() const -> std::vector<std::string>
   {
      return native_database_.getAllFiles();
   }
}