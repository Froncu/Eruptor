#pragma once

#include <clang/Frontend/FrontendAction.h>

namespace tep
{
   class Action final : public clang::ASTFrontendAction
   {
      public:
         Action() = default;
         Action(Action const&) = delete;
         Action(Action&&) = delete;

         ~Action() override = default;

         auto operator=(Action const&) -> Action& = delete;
         auto operator=(Action&&) -> Action& = delete;

      protected:
         auto CreateASTConsumer(clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer> override;
   };
}