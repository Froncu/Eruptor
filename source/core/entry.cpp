#include "eruptor/eruptor.hpp"

namespace eru
{
   [[nodiscard]] auto create_application(std::span<char const* const> arguments) -> std::unique_ptr<Application>;
}

auto main(int const arguments_count, char const* const* arguments) -> int try
{
   std::unique_ptr const application{ eru::create_application({ arguments, static_cast<std::size_t>(arguments_count) }) };

   while (application->tick())
      application->poll();

   return 0;
}
catch (eru::Exception const& exception)
{
   eru::Locator::provide<eru::Logger>().error(exception.what(), false, exception.source_location());
   eru::Locator::remove_all();
   return 1;
}