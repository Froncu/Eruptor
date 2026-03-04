#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "eruptor/api.hpp"
#include "eruptor/constants.hpp"
#include "eruptor/pch.hpp"

namespace std
{
   template <>
   struct hash<source_location>
   {
      [[nodiscard]] std::size_t operator()(source_location const& location) const noexcept;
   };

   template <>
   struct equal_to<source_location>
   {
      [[nodiscard]] bool operator()(source_location const& location_a, source_location const& location_b) const noexcept;
   };
}

namespace eru
{
   class Logger final
   {
      enum class Type
      {
         INFO,
         WARNING,
         ERROR
      };

      struct Payload final
      {
         Type type;
         bool framework_level;
         std::source_location location;
         std::string message;
      };

      struct LogInfo final
      {
         bool once;
         Payload payload;
      };

      public:
         ERU_API Logger();
         Logger(Logger const&) = delete;
         Logger(Logger&&) = delete;

         ERU_API ~Logger();

         Logger& operator=(Logger const&) = delete;
         Logger& operator=(Logger&&) = delete;

         ERU_API void register_engine_source_root(std::filesystem::path user_root);
         ERU_API void register_source_root(std::filesystem::path user_root, std::filesystem::path compile_root);

         template <typename Message>
         void info(Message&& message, bool const once = false, std::source_location location = std::source_location::current())
         {
            {
               std::lock_guard const lock{ mutex_ };
               log_queue_.push({
                  .once{ once },
                  .payload{
                     .type{ Type::INFO },
                     .framework_level{ FRAMEWORK_LEVEL },
                     .location{ std::move(location) },
                     .message{ std::format("{}", message) }
                  }
               });
            }

            condition_.notify_one();
         }

         template <typename Message>
         void warning(Message&& message, bool const once = false,
            std::source_location location = std::source_location::current())
         {
            {
               std::lock_guard const lock{ mutex_ };
               log_queue_.push({
                  .once{ once },
                  .payload{
                     .type{ Type::WARNING },
                     .framework_level{ FRAMEWORK_LEVEL },
                     .location{ std::move(location) },
                     .message{ std::format("{}", message) }
                  }
               });
            }

            condition_.notify_one();
         }

         template <typename Message>
         void error(Message&& message, bool const once = false, std::source_location location = std::source_location::current())
         {
            {
               std::lock_guard const lock{ mutex_ };
               log_queue_.push({
                  .once{ once },
                  .payload{
                     .type{ Type::ERROR },
                     .framework_level{ FRAMEWORK_LEVEL },
                     .location{ std::move(location) },
                     .message{ std::format("{}", message) }
                  }
               });
            }

            condition_.notify_one();
         }

      private:
          void log(Payload const& payload);
          void log_once(Payload const& payload);

         std::vector<std::pair<std::filesystem::path, std::filesystem::path>> source_roots_;
         std::unordered_set<std::source_location> location_entries_{};

         std::queue<LogInfo> log_queue_{};

         std::mutex mutex_{};
         std::condition_variable condition_{};
         std::jthread thread_{
            [this](std::stop_token const& stop_token)
            {
               LogInfo log_info;
               while (true)
               {
                  {
                     std::unique_lock lock{ mutex_ };
                     condition_.wait(lock,
                        [this, &stop_token]
                        {
                           return not log_queue_.empty() or stop_token.stop_requested();
                        });

                     if (log_queue_.empty())
                        break;

                     log_info = std::move(log_queue_.front());
                     log_queue_.pop();
                  }

                  if (log_info.once)
                     log_once(log_info.payload);
                  else
                     log(log_info.payload);
               }
            }
         };
   };
}

#endif