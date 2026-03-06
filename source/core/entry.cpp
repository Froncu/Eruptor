#include "eruptor/eruptor.hpp"

namespace eru
{
   [[nodiscard]] std::unique_ptr<Application> create_application(std::span<char const* const> arguments);
}

int main(int const arguments_count, char const* const* arguments)
{
   std::unique_ptr const application{ eru::create_application({ arguments, static_cast<std::size_t>(arguments_count) }) };

   while (application->tick())
      application->poll();

   return 0;
}