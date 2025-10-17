#include "window.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	ke::Window::init();
#ifndef NDEBUG
	ke::Logger logger("Main Function Logger", spdlog::level::trace);
#endif
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
	unsigned int screenWidth = videoMode->width, screenHeight = videoMode->height;

	ke::Window window(screenWidth/2, screenHeight/2, "Hello, World!");
	window.setPosition(screenWidth / 4, screenHeight / 4);
#ifndef NDEBUG
	logger.info("Created GLFW window.");
#endif
	ke::Renderer& renderer = ke::Renderer::getInstance();
#ifndef NDEBUG
	logger.trace("Called for vulkan initiation.");
#endif
	renderer.initVulkan(window.getWindow());
#ifndef NDEBUG
	logger.info("Finished Vulkan initiation.");
#endif
	while (!window.shouldClose())
	{
		renderer.beginRecording(window.getWindow(), window.hasResized());
		// DRAW CALLS GO HERE
		vkCmdDraw(renderer.getCommandBuffer(), 3, 1, 0, 0);
		
		renderer.endRecording();
		renderer.present(window.getWindow());

		window.pollEvents();
		renderer.advanceFrame();
	}	

	renderer.cleanupRenderer();
}