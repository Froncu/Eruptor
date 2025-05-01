#ifndef RENDER_PASS_BUILDER_HPP
#define RENDER_PASS_BUILDER_HPP

#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"

namespace eru
{
   class RenderPassBuilder final
   {
      public:
         struct ColorResolveAttachmentReference final
         {
            vk::AttachmentReference color_attachment_reference{};
            vk::AttachmentReference resolve_attachment_reference{ vk::AttachmentUnused };
         };

         struct Subpass final
         {
            vk::PipelineBindPoint pipeline_bind_point{};
            std::vector<vk::AttachmentReference> input_attachment_references{};
            std::vector<std::uint32_t> preserve_attachment_references{};
            std::vector<ColorResolveAttachmentReference> color_resolve_attachment_references{};
            vk::AttachmentReference depth_stencil_attachment_reference{};
         };

         RenderPassBuilder() = default;
         RenderPassBuilder(RenderPassBuilder const&) = delete;
         RenderPassBuilder(RenderPassBuilder&&) = delete;

         ~RenderPassBuilder() = default;

         RenderPassBuilder& operator=(RenderPassBuilder const&) = delete;
         RenderPassBuilder& operator=(RenderPassBuilder&&) = delete;

         RenderPassBuilder& add_attachment(vk::AttachmentDescription const& attachment);
         RenderPassBuilder& add_attachments(std::span<vk::AttachmentDescription const> attachments);
         RenderPassBuilder& add_subpass(Subpass const& subpass);
         RenderPassBuilder& add_subpasses(std::span<Subpass const> subpasses);
         RenderPassBuilder& add_subpass_dependency(vk::SubpassDependency const& subpass_dependency);
         RenderPassBuilder& add_subpass_dependencies(std::span<vk::SubpassDependency const> subpass_dependencies);

         [[nodiscard]] RenderPass build(Device const& device);

      private:
         std::vector<vk::AttachmentDescription> attachments_{};
         std::vector<Subpass> subpasses_{};
         std::vector<vk::SubpassDependency> subpass_dependencies_{};
   };
}

#endif