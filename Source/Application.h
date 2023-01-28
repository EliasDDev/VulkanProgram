#pragma once

#include <string>

struct GLFWwindow;

namespace Vp
{
	class VpRenderer;

	class Application
	{
	public:
		Application(const int& width, const int& height);
		~Application();

		void PollEvents();

		int GetWidth() const { return m_Width; }
		void SetWidth(const int& width);

		int GetHeight() const { return m_Height; }
		void SetHeight(const int& height);

		std::string GetTitle() const { return m_Title; }
		void SetTitle(const std::string& title);

		bool UserRequstedQuit() const { return m_UserRequstedQuit; }
		void Quit() { m_UserRequstedQuit = true; }

		GLFWwindow* GetNativeWindow() const { return m_Window; }
	private:
		int m_Width = {};
		int m_Height = {};
		std::string m_Title = {};
		bool m_UserRequstedQuit = false;

		GLFWwindow* m_Window = nullptr;

		Vp::VpRenderer* m_Renderer = nullptr;
	};
}
