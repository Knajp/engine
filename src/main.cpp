#include "window.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	ke::Window::init();
	ke::Logger logger("Main Function Logger", spdlog::level::trace);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
	unsigned int screenWidth = videoMode->width, screenHeight = videoMode->height;

	ke::Window window(screenWidth/2, screenHeight/2, "Hello, World!");
	window.setPosition(screenWidth / 4, screenHeight / 4);
	logger.info("Created GLFW window.");

	ke::Renderer& renderer = ke::Renderer::getInstance();
	logger.trace("Called for vulkan initiation.");
	renderer.initVulkan(window.getWindow());

	while (!window.shouldClose())
	{

		window.pollEvents();
	}	

	renderer.cleanupRenderer();
}