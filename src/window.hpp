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

		static void init();

		void setPosition(uint16_t x, uint16_t y);

		bool shouldClose();

		void pollEvents();

		GLFWwindow* getWindow() const;
	private:
		const uint16_t mWidth;
		const uint16_t mHeight;

		GLFWwindow* pWindow;
		std::string mWindowName;

	private:
		void initWindow();
	};
}