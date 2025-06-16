# Eruptor - Vulkan PBR Renderer

A Physically Based Rendering (PBR) engine built in C++ using Vulkan.

## Features

- ✅ **Physically Based Rendering (PBR)**  
  Realistic lighting and material representation using PBR techniques.

- ✅ **Deferred Rendering Pipeline**  
  Efficient lighting using a deferred shading setup with support for tone-mapping.

- ✅ **Automatic Exposure (Eye Adaptation)**  
  Real-time luminance adaptation using **compute shaders** and the **histogram-based technique**.  
  Adjustable via exposed parameters for controlling adaptation speed and the luminance range.

- ✅ **Modern Vulkan Techniques**
  
  - **Dynamic Rendering**
  - **Bindless Rendering** via descriptor indexing
  - **Synchronization2** for explicit and flexible synchronization

- ✅ **C++20 & VulkanHPP**  
  Type-safe and RAII-compliant Vulkan API via the Vulkan-Hpp C++ bindings.

- ✅ **RAII-Oriented Design**  
  Every Vulkan resource is wrapped in a safe, scope-managed class to eliminate leaks and ensure correctness.

- ✅ **Builder Pattern**  
  Resource creation and pipeline configuration are managed using clean, chainable builders for clarity and reusability.

---

## Author

**Jakub Fratczak**  
🔗 https://github.com/Howest-DAE-GD/Eruptor
