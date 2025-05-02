#include "eruptor.hpp"

namespace eru
{
   void Eruptor::run()
   {
      while (is_running_)
      {
         Locator::get<SystemEventDispatcher>().poll_events();
      }
   }
}