#include "eruptor.hpp"

namespace eru
{
   Eruptor::Eruptor()
   {
      window_.change_resizability(true);
   }

   void Eruptor::run()
   {
      while (is_running_)
      {
         Locator::get<SystemEventDispatcher>().poll_events();
         renderer_.render();
      }
   }
}