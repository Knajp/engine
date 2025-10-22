#include "window.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include <iostream>
#include "Rect.hpp"
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

	{
		ke::prim::Rect rectangle({ -0.5f, -0.5f }, { 1.0f, 1.0f });

		while (!window.shouldClose())
		{
			renderer.beginRecording(window.getWindow(), window.hasResized());
			// DRAW CALLS GO HERE
			rectangle.Draw(renderer.getCommandBuffer());

			renderer.endRecording();
			renderer.present(window.getWindow());


			renderer.destroyRedundantBuffers();

			window.pollEvents();
			renderer.advanceFrame();
		}
	}
	   
	renderer.cleanupRenderer();
}