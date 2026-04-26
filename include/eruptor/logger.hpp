#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "eruptor/api.hpp"
#include "eruptor/constants.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/pch.hpp"

namespace std
{
   template <>
   struct hash<source_location>
   {
      [[nodiscard]] auto operator()(source_location const& location) const noexcept -> std::size_t;
   };

   template <>
   struct equal_to<source_location>
   {
      [[nodiscard]] auto operator()(source_location const& location_a, source_location const& location_b) const noexcept -> bool;
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
         ERU_API explicit Logger(Locator::ConstructionKey);
         Logger(Logger const&) = delete;
         Logger(Logger&&) = delete;

         ERU_API ~Logger();

         auto operator=(Logger const&) -> Logger& = delete;
         auto operator=(Logger&&) -> Logger& = delete;

         ERU_API auto register_framework_source_root(std::filesystem::path user_root) -> void;
         ERU_API auto register_source_root(std::filesystem::path user_root, std::filesystem::path compile_root) -> void;

         template <typename Message>
         auto info(Message&& message, bool const once = false, std::source_location location = std::source_location::current()) -> void
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
         auto warning(Message&& message, bool const once = false,
            std::source_location location = std::source_location::current()) -> void
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
         auto error(Message&& message, bool const once = false, std::source_location location = std::source_location::current()) -> void
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
         auto log(Payload const& payload) -> void;
         auto log_once(Payload const& payload) -> void;

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