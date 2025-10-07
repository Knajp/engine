#include "window.hpp"
#include "logger.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	ke::Logger logger("Main Function Logger", spdlog::level::trace);

	ke::Window window(800, 800, "Hello, World!");
	logger.info("Created GLFW window.");

	while (!window.shouldClose())
	{

		window.pollEvents();
		logger.trace("Event poll finished.");
	}
}