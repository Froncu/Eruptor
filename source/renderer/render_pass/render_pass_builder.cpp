#include "render_pass_builder.hpp"

namespace eru
{
   RenderPassBuilder& RenderPassBuilder::add_attachment(vk::AttachmentDescription const& attachment)
   {
      attachments_.push_back(attachment);
      return *this;
   }

   RenderPassBuilder& RenderPassBuilder::add_attachments(std::span<vk::AttachmentDescription const> const attachments)
   {
      attachments_.insert(attachments_.end(), attachments.begin(), attachments.end());
      return *this;
   }

   RenderPassBuilder& RenderPassBuilder::add_subpass(Subpass const& subpass)
   {
      subpasses_.push_back(subpass);
      return *this;
   }

   RenderPassBuilder& RenderPassBuilder::add_subpasses(std::span<Subpass const> subpasses)
   {
      subpasses_.insert(subpasses_.end(), subpasses.begin(), subpasses.end());
      return *this;
   }

   RenderPassBuilder& RenderPassBuilder::add_subpass_dependency(vk::SubpassDependency const& subpass_dependency)
   {
      subpass_dependencies_.push_back(subpass_dependency);
      return *this;
   }

   RenderPassBuilder& RenderPassBuilder::add_subpass_dependencies(std::span<vk::SubpassDependency const> subpass_dependencies)
   {
      subpass_dependencies_.insert(subpass_dependencies_.end(), subpass_dependencies.begin(), subpass_dependencies.end());
      return *this;
   }

   RenderPass RenderPassBuilder::build(Device const& device)
   {
      std::vector<vk::SubpassDescription> subpass_descriptions{};
      subpass_descriptions.reserve(subpasses_.size());

      std::vector<std::vector<vk::AttachmentReference>> color_attachment_references{};
      std::vector<std::vector<vk::AttachmentReference>> resolve_attachment_references{};

      for (auto const& [pipeline_bind_point,
              input_attachment_references, preserve_attachment_references,
              color_resolve_attachment_references, depth_stencil_attachment_reference] : subpasses_)
      {
         color_attachment_references.push_back({});
         resolve_attachment_references.push_back({});

         for (auto const& [color_attachment_reference, resolve_attachment_reference] :
              color_resolve_attachment_references)
         {
            color_attachment_references.back().push_back(color_attachment_reference);
            resolve_attachment_references.back().push_back(resolve_attachment_reference);
         }

         subpass_descriptions.push_back({
            .pipelineBindPoint{ pipeline_bind_point },
            .inputAttachmentCount{ static_cast<std::uint32_t>(input_attachment_references.size()) },
            .pInputAttachments{ input_attachment_references.data() },
            .colorAttachmentCount{ static_cast<std::uint32_t>(color_resolve_attachment_references.size()) },
            .pColorAttachments{ color_attachment_references.back().data() },
            .pResolveAttachments{ resolve_attachment_references.back().data() },
            .pDepthStencilAttachment{ &depth_stencil_attachment_reference },
            .preserveAttachmentCount{ static_cast<std::uint32_t>(preserve_attachment_references.size()) },
            .pPreserveAttachments{ preserve_attachment_references.data() }
         });
      }

      return device.device().createRenderPass({
         .attachmentCount{ static_cast<std::uint32_t>(attachments_.size()) },
         .pAttachments{ attachments_.data() },
         .subpassCount{ static_cast<std::uint32_t>(subpass_descriptions.size()) },
         .pSubpasses{ subpass_descriptions.data() },
         .dependencyCount{ static_cast<std::uint32_t>(subpass_dependencies_.size()) },
         .pDependencies{ subpass_dependencies_.data() }
      });
   }
}