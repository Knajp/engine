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

void ke::Window::init()
{
	glfwInit();
}

void ke::Window::setPosition(uint16_t x, uint16_t y)
{
	glfwSetWindowPos(pWindow, x, y);
}

bool ke::Window::shouldClose()
{
	return glfwWindowShouldClose(pWindow);
}

void ke::Window::pollEvents()
{
	glfwPollEvents();
}

GLFWwindow* ke::Window::getWindow() const
{
	return pWindow;
}

void ke::Window::initWindow()
{

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	pWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
}
