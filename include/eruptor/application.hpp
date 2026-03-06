#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Application
   {
      class GLFWcontext final
      {
         public:
            GLFWcontext();
            GLFWcontext(GLFWcontext const&) = delete;
            GLFWcontext(GLFWcontext&&) = delete;

            ~GLFWcontext();

            GLFWcontext& operator=(GLFWcontext&&) = delete;
            GLFWcontext& operator=(GLFWcontext&) = delete;
      };

      public:
         Application(Application const&) = delete;
         Application(Application&&) noexcept = delete;

         ERU_API virtual ~Application();

         Application& operator=(Application const&) = delete;
         Application& operator=(Application&&) = delete;

         [[nodiscard]] virtual bool tick() = 0;
         ERU_API [[nodiscard]] void poll();

      protected:
         ERU_API explicit Application();

      private:
         GLFWcontext const glfw_context_{};
   };
}

#endif