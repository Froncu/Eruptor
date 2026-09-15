#include <print>

#include <clang/Tooling/AllTUsExecution.h>

#include "action_factory.hpp"
#include "compilation_database.hpp"

auto main() -> int
{
   std::string database_error;
   std::unique_ptr const database{ clang::tooling::CompilationDatabase::autoDetectFromDirectory(".", database_error) };
   if (not database)
   {
      std::println("{}", database_error);
      return 1;
   }

   tep::CompilationDatabase const filtered_database{ *database };
   clang::tooling::AllTUsToolExecutor tool{ filtered_database, 0 };
   if (llvm::Error error{ tool.execute(std::make_unique<tep::ActionFactory>()) })
   {
      logAllUnhandledErrors(std::move(error), llvm::errs(), "tephra: ");
      return 1;
   }

   return 0;
}