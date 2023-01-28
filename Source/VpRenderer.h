#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Vp
{
	class VpRenderer
	{
	public:
		VpRenderer(GLFWwindow* window);
		~VpRenderer();
	private:
		GLFWwindow* m_Window = nullptr;
	};
}
