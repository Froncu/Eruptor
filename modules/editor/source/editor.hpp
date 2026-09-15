#pragma once

#include <eruptor/core/application.hpp>
#include <eruptor/core/platform.hpp>
#include <eruptor/core/renderer.hpp>

namespace mgm
{
   class Editor final : public eru::Application
   {
      public:
         explicit Editor(eru::PassKey<eru::Locator> construction_key);
         Editor(Editor const&) = delete;
         Editor(Editor&&) noexcept = delete;

         ~Editor() override = default;

         auto operator=(Editor const&) -> Editor& = delete;
         auto operator=(Editor&&) noexcept -> Editor& = delete;

         [[nodiscard]] auto tick() -> bool override;

      private:
         eru::Platform const& platform_{ eru::Locator::get<eru::Platform>() };
         eru::Window window_{};
         eru::Renderer renderer_{};
   };
}