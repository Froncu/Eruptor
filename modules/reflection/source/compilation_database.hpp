#pragma once

#include <clang/Tooling/CompilationDatabase.h>

namespace tep
{
   class CompilationDatabase final : public clang::tooling::CompilationDatabase
   {
      public:
         explicit CompilationDatabase(clang::tooling::CompilationDatabase const& native_database);
         CompilationDatabase(CompilationDatabase const&) = default;
         CompilationDatabase(CompilationDatabase&&) = default;

         ~CompilationDatabase() override = default;

         auto operator=(CompilationDatabase const&) -> CompilationDatabase& = delete;
         auto operator=(CompilationDatabase&&) -> CompilationDatabase& = delete;

         auto getCompileCommands(llvm::StringRef file_path) const -> std::vector<clang::tooling::CompileCommand> override;
         auto getAllFiles() const -> std::vector<std::string> override;

      private:
         clang::tooling::CompilationDatabase const& native_database_;
   };
}