#include "eruptor/eruptor.hpp"

namespace eru
{
   auto create_application(std::span<char const* const> arguments) -> void;
}

auto main(int const arguments_count, char const* const* arguments) -> int try
{
   eru::Locator::provide<eru::Logger>();
   eru::create_application({ arguments, static_cast<std::size_t>(arguments_count) });

   eru::Application& application{ eru::Locator::get<eru::Application>() };
   while (application.tick())
      application.poll();

   eru::Locator::remove_all();
   return 0;
}
catch (eru::Exception const& exception)
{
   eru::Locator::get<eru::Logger>().error(exception.what(), false, exception.source_location());
   eru::Locator::remove_all();
   return 1;
}