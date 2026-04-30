#include "eruptor/eruptor.hpp"

namespace eru
{
   auto provide_application(std::span<char const* const> arguments) -> void;
}

auto main(int const arguments_count, char const* const* arguments) -> int try
{
   eru::Locator::provide<eru::Logger>();
   eru::Locator::provide<eru::Platform>();
   eru::Locator::provide<eru::Renderer>();
   eru::provide_application({ arguments, static_cast<std::size_t>(arguments_count) });

   while (eru::Locator::get<eru::Application>().tick())
   {
   }

   eru::Locator::remove_all();
   return 0;
}
catch (eru::Exception const& exception)
{
   eru::Locator::get<eru::Logger>().error(exception.what(), false, exception.source_location());
   eru::Locator::remove_all();
   return 1;
}