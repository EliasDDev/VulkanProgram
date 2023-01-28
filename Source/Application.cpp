#include "Application.h"

#include "VpRenderer.h"

Vp::Application::Application(const int& width, const int& height)
	: m_Width(width), m_Height(height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(m_Width, m_Height, nullptr, nullptr, nullptr);

	m_Renderer = new Vp::VpRenderer(m_Window);
}

Vp::Application::~Application()
{
	delete m_Renderer;

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}
