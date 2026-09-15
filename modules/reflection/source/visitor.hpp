#pragma once

#include <clang/AST/RecursiveASTVisitor.h>

namespace tep
{
   class Visitor final : public clang::RecursiveASTVisitor<Visitor>
   {
      public:
         explicit Visitor(clang::SourceManager const& source_manager);
         Visitor(Visitor const&) = default;
         Visitor(Visitor&&) = default;

         ~Visitor() = default;

         auto operator=(Visitor const&) -> Visitor& = delete;
         auto operator=(Visitor&&) -> Visitor& = delete;

         auto VisitRecordDecl(clang::RecordDecl const* declaration) const -> bool;

      private:
         clang::SourceManager const& source_manager_;
   };
}