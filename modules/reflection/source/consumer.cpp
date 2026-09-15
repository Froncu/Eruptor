#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclGroup.h>

#include "consumer.hpp"

namespace tep
{
   Consumer::Consumer(clang::SourceManager const& source_manager)
      : visitor_{ source_manager }
   {
   }

   auto Consumer::HandleTopLevelDecl(clang::DeclGroupRef declarations) -> bool
   {
      for (clang::Decl* const declaration : declarations)
         visitor_.TraverseDecl(declaration);

      return true;
   }
}