#include "window.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	ke::Logger logger("Main Function Logger", spdlog::level::trace);

	ke::Window window(800, 800, "Hello, World!");
	logger.info("Created GLFW window.");

	ke::Renderer& renderer = ke::Renderer::getInstance();
	logger.trace("Called for vulkan initiation.");
	renderer.initVulkan();

	while (!window.shouldClose())
	{

		window.pollEvents();
	}	

	renderer.cleanupRenderer();
}