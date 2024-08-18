#include <iostream>

#include <vulkan/vulkan.hpp>

int main()
{
	auto instance{ vk::createInstance(vk::InstanceCreateInfo{}) };
}