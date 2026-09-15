#include "action.hpp"
#include "action_factory.hpp"

namespace tep
{
   auto ActionFactory::create() -> std::unique_ptr<clang::FrontendAction>
   {
      return std::make_unique<Action>();
   }
}