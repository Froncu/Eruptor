#include "action.hpp"
#include "consumer.hpp"

namespace tep
{
   auto Action::CreateASTConsumer(clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer>
   {
      return std::make_unique<Consumer>(compiler.getSourceManager());
   }
}