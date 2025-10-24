#include "window.hpp"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto windowClass = reinterpret_cast<ke::Window*>(glfwGetWindowUserPointer(window));
	windowClass->setResized();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

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
	std::cout << "Poll events\n";
	glfwPollEvents();
}

GLFWwindow* ke::Window::getWindow() const
{
	return pWindow;
}

bool ke::Window::hasResized() const
{
	return mHasResized;
}

void ke::Window::setResized()
{
	mHasResized = true;
}

void ke::Window::processInput()
{
	
	if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double Xpos, Ypos;
		glfwGetCursorPos(pWindow, &Xpos, &Ypos);

		int width, height;
		glfwGetWindowSize(pWindow, &width, &height);

		float ndcX = (2.0f * Xpos) / static_cast<float>(width) - 1.0f;
		float ndcY = (2.0f * Ypos) / static_cast<float>(height) - 1.0f;

		for (auto& el : mInteractable)
		{
			if (el->isHovering({ ndcX, ndcY }))
				
				el->onClick();
		}
	}
}

void ke::Window::addToInteractable(std::shared_ptr<ke::ui::Button> element)
{
	mInteractable.push_back(element);
}

void ke::Window::clearInteractable()
{
	mInteractable.clear();
}

void ke::Window::initWindow()
{

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);

	pWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(pWindow, this);
	glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);

	glfwSetWindowFocusCallback(pWindow, [](GLFWwindow* window, int focused) {
		if (!focused) return
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		std::cout << "Focused window\n";
		});

	glfwSetWindowIconifyCallback(pWindow, [](GLFWwindow* window, int iconified) {
		if (iconified) return;
		glfwFocusWindow(window); // Important
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		std::cout << "Restored window\n";

		});
}
