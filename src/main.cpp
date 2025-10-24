#include "window.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include <iostream>
#include "Rect.hpp"
#include "graphicalInterface.hpp"

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
		auto exitButton = std::make_shared<ke::ui::Button>(glm::vec2{ 0.9f, -1.0f }, glm::vec2{ 0.1f, 0.1f }, glm::vec3{ 0.2f, 0.2f, 0.2f });
		exitButton->onClick = [&window]()
			{
				glfwSetWindowShouldClose(window.getWindow(), true);
			};

		auto minimizeButton = std::make_shared<ke::ui::Button>(glm::vec2{ 0.79f, -1.0f }, glm::vec2{ 0.1f, 0.1f }, glm::vec3{ 0.2f, 0.2f, 0.2f });
		minimizeButton->onClick = [&window]()
			{
				glfwIconifyWindow(window.getWindow());
			};
		window.addToInteractable(minimizeButton);
		window.addToInteractable(exitButton);

		while (!window.shouldClose())
		{
			window.processInput();

			renderer.beginRecording(window.getWindow(), window.hasResized());
			// DRAW CALLS GO HERE
			exitButton->Draw(renderer.getCommandBuffer());
			minimizeButton->Draw(renderer.getCommandBuffer());

			renderer.endRecording();
			renderer.present(window.getWindow());

			renderer.destroyRedundantBuffers();

			window.pollEvents();
			renderer.advanceFrame();
		}

		window.clearInteractable();
	}
	   
	renderer.cleanupRenderer();
}