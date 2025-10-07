#pragma once
#include <iostream>
#include <GLFW/glfw3.h>

namespace ke 
{
	class Window
	{
	public:
		Window(uint16_t w, uint16_t h, std::string n);
		~Window();

		bool shouldClose();

		void pollEvents();
	private:
		const uint16_t mWidth;
		const uint16_t mHeight;

		GLFWwindow* pWindow;
		std::string mWindowName;

	private:
		void initWindow();
	};
}