# Eruptor

A PBR renderer built in C++20 using modern Vulkan practices. Serves as a way of exploring the world of Graphics Programming.

## Features

- **Physically Based Rendering (PBR)**  
Realistic lighting and material representation using PBR techniques.

- **Deferred Rendering Pipeline**  
Efficient lighting using a deferred shading setup with support for tone-mapping.

- **Automatic Exposure (Eye Adaptation)**  
Real-time luminance adaptation using compute shaders and the histogram-based technique. Adjustable via exposed parameters for controlling adaptation speed and the luminance range.

- **RAII-Oriented Design**  
Every Vulkan resource is wrapped in a safe, scope-managed class to eliminate leaks and ensure correctness.

- **Builder Pattern**  
Resource creation and pipeline configuration are managed using clean, chainable builders for clarity and reusability.

- **C++20 & VulkanHPP**  
Type-safe and RAII-compliant Vulkan API via the Vulkan-Hpp C++ bindings.

- **Dynamic Rendering**  
Renders without traditional render passes for simplified, flexible pipelines.

- **Bindless Rendering**  
Accesses many resources in shaders without frequent descriptor set updates.

- **Synchronization2**  
Explicit, fine-grained synchronization with simplified barriers and dependencies

## Future Plans

- **Image-Based Lighting (IBL)**  
Add environment-based lighting for more realistic material shading.

- **Shadows**  
Implement shadow mapping for directional & point lights.


## Dependencies

- [Vulkan](https://vulkan.lunarg.com/sdk/home) - Obviously
- [SDL3](https://github.com/libsdl-org/SDL) - Core functionality (entry point, system events, input, windowing, etc.)
- [SDL3_image](https://github.com/libsdl-org/SDL_image) - Image loading support
- [GLM](https://github.com/g-truc/glm) - Math library
- [assimp](https://github.com/assimp/assimp) - Scene loading
- [spdlog](https://github.com/gabime/spdlog) - Logging

Vulkan is the only dependency that must be installed manually. All other dependencies are managed through my custom [vcpkg](https://github.com/microsoft/vcpkg) fork and will be automatically installed during the configuration process.

## Quickstart

### Prerequisites

- C++20 compatible compiler
- CMake 3.15 or higher
- Vulkan SDK 1.4.328.1

### Cloning

Clone the repository **with** the `--recursive` flag. This will cause the vcpkg submodule to also be pulled:

```bash
git clone https://github.com/Froncu/Eruptor --recursive
```

### Configuring & Building

The project includes CMake presets (`CMakePresets.json`) that configure vcpkg integration automatically. You can configure and build using your preferred IDE or command line.

**Note:** If creating custom configurations, ensure they inherit from the provided `base`, `debug` and/or `release` preset to maintain proper vcpkg integration.

### Usage

You can navigate the scene using either a keyboard and mouse or a gamepad.
- **Keyboard & Mouse**
  - W/A/S/D – Move around.
  - Mouse – Look around.
  - Up/Down arrows – Increase or decrease movement speed.
  - Shift/Ctrl – Move vertically up or down.
- **Gamepad**:
  - Left joystick – Move around.
  - Right joystick – Look around.
  - West/South buttons – Increase or decrease movement speed.
  - Right/Left triggers – Move vertically up or down.