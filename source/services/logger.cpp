#include "eruptor/hash.hpp"
#include "eruptor/logger.hpp"

namespace std
{
   size_t hash<source_location>::operator()(source_location const& location) const noexcept
   {
      size_t const hash_1{ eru::hash(location.file_name()) };
      size_t const hash_2{ eru::hash(location.line()) };
      size_t const hash_3{ eru::hash(location.column()) };
      size_t const hash_4{ eru::hash(location.function_name()) };

      size_t seed{};
      auto const generate{
         [&seed](size_t const hash)
         {
            seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
         }
      };

      generate(hash_1);
      generate(hash_2);
      generate(hash_3);
      generate(hash_4);

      return seed;
   }

   bool equal_to<source_location>::operator()(source_location const& location_a,
      source_location const& location_b) const noexcept
   {
      if (location_a.line() not_eq location_b.line() or
         location_a.column() not_eq location_b.column())
         return false;

      if (location_a.file_name() == location_b.file_name() and
         location_a.function_name() == location_b.function_name())
         return true;

      return
         not std::strcmp(location_a.file_name(), location_b.file_name()) and
         not std::strcmp(location_a.function_name(), location_b.function_name());
   }
}

namespace eru
{
   Logger::Logger(Locator::ConstructionKey)
   {
   }

   Logger::~Logger()
   {
      thread_.request_stop();
      condition_.notify_one();
   }

   void Logger::register_framework_source_root(std::filesystem::path user_root)
   {
      register_source_root(std::move(user_root), COMPILE_SOURCE_PATH.data());
   }

   void Logger::register_source_root(std::filesystem::path user_root, std::filesystem::path compile_root)
   {
      if (not exists(user_root))
         throw Exception{ "the specified user source root must exist" };

      if (compile_root.empty())
         throw Exception{ "the specified compile root cannot be empty" };

      source_roots_.emplace_back(std::move(user_root), std::move(compile_root));
   }

   void Logger::log(Payload const& payload)
   {
      std::ostream* output_stream;
      switch (payload.type)
      {
         case Type::INFO:
            output_stream = &std::clog;
            break;

         case Type::WARNING:
            [[fallthrough]];

         case Type::ERROR:
            output_stream = &std::cerr;
            break;

         default:
            output_stream = &std::cout;
      }

      std::string_view escape_sequence;
      std::string_view type;
      switch (payload.type)
      {
         case Type::INFO:
            escape_sequence = "1;36";
            type = "INFO";
            break;

         case Type::WARNING:
            escape_sequence = "1;33";
            type = "WARNING";
            break;

         case Type::ERROR:
            escape_sequence = "1;31";
            type = "ERROR";
            break;
      }

      std::filesystem::path path{ payload.location.file_name() };
      for (auto const& [user_root, compile_root] : source_roots_)
         if (std::ranges::mismatch(compile_root, path).in1 == compile_root.end())
         {
            path = user_root / relative(path, compile_root);
            break;
         }

      std::string source_file_location;
      if (exists(path))
         source_file_location = std::format("{}({})", path.lexically_normal().string(), payload.location.line());
      else
         source_file_location = "unknown source file location";

      std::println(*output_stream,
         "[\x1b[{}m{}\x1b[0m] [\x1b[1;97m{}\x1b[0m] [\x1b[1;97m{}\x1b[0m]\n{}",
         escape_sequence,
         type,
         payload.framework_level ? "ERUPTOR" : "APP",
         source_file_location,
         payload.message);
   }

   void Logger::log_once(Payload const& payload)
   {
      if (not location_entries_.insert(payload.location).second)
         return;

      return log(payload);
   }
}