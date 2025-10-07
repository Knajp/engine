#include "window.hpp"

ke::Window::Window(uint16_t w, uint16_t h, std::string n)
	:mWidth(w), mHeight(h), mWindowName(n)
{
	initWindow();
}

ke::Window::~Window()
{
	glfwDestroyWindow(pWindow);
	glfwTerminate();
}

bool ke::Window::shouldClose()
{
	return glfwWindowShouldClose(pWindow);
}

void ke::Window::pollEvents()
{
	glfwPollEvents();
}

void ke::Window::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	pWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
}
