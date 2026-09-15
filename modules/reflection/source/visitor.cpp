#include <print>

#include <clang/Basic/SourceManager.h>

#include "visitor.hpp"

namespace tep
{
   Visitor::Visitor(clang::SourceManager const& source_manager)
      : source_manager_{ source_manager }
   {
   }

   auto Visitor::VisitRecordDecl(clang::RecordDecl const* const declaration) const -> bool
   {
      if (clang::SourceLocation const location{ source_manager_.getExpansionLoc(declaration->getLocation()) };
         not declaration->isStruct() or location.isInvalid() or source_manager_.isLoadedSourceLocation(location))
         return true;

      for (clang::AnnotateAttr const* annotation : declaration->specific_attrs<clang::AnnotateAttr>())
         if (annotation->getAnnotation() == "shader_structure")
            std::println("found: {}", declaration->getNameAsString());

      return true;
   }
}