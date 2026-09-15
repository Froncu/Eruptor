#pragma once

#include <clang/Tooling/Tooling.h>

namespace tep
{
   class ActionFactory final : public clang::tooling::FrontendActionFactory
   {
      public:
         ActionFactory() = default;
         ActionFactory(ActionFactory const&) = default;
         ActionFactory(ActionFactory&&) = default;

         ~ActionFactory() override = default;

         auto operator=(ActionFactory const&) -> ActionFactory& = default;
         auto operator=(ActionFactory&&) -> ActionFactory& = default;

         auto create() -> std::unique_ptr<clang::FrontendAction> override;
   };
}