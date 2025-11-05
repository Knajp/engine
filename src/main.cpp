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
		auto topBar = std::make_shared<ke::ui::GUIelement>(glm::vec2( -1.0f, -1.0f ), glm::vec2(2.0f, 0.15f), glm::vec3(0.05f, 0.05f, 0.05f));

		auto exitButton = std::make_shared<ke::ui::Button>(glm::vec2{ 0.9f, -1.0f }, glm::vec2{ 0.1f, 0.15f }, glm::vec3{ 0.045f, 0.045f, 0.045f });
		exitButton->onClick = [&window]()
			{
				glfwSetWindowShouldClose(window.getWindow(), true);
			};

		auto minimizeButton = std::make_shared<ke::ui::Button>(glm::vec2{ 0.79f, -1.0f }, glm::vec2{ 0.1f, 0.15f }, glm::vec3{ 0.045f, 0.045f, 0.045f });
		minimizeButton->onClick = [&window]()
			{
				glfwIconifyWindow(window.getWindow());
			};

		topBar->addToChildren(minimizeButton);
		topBar->addToChildren(exitButton);

		auto rightBar = std::make_shared<ke::ui::GUIelement>(glm::vec2(0.5f, -0.85f), glm::vec2(0.5f, 1.85f), glm::vec3(0.055f, 0.055f, 0.055f));

		auto tabs = std::make_shared<ke::ui::GUIelement>(glm::vec2(-1.0f, -0.85f), glm::vec2(1.5f, 0.10), glm::vec3(0.055f, 0.055f, 0.055f));
		auto overviewButton = std::make_shared<ke::ui::Button>(glm::vec2{ -1.0f + 0.0f * 0.3775f, -0.85f }, glm::vec2{ 0.3675f, 0.1f }, glm::vec3{ 0.045f, 0.045f, 0.045f });
		auto changelogButton = std::make_shared<ke::ui::Button>(glm::vec2{ -1.0f + 1.0f * 0.3775f, -0.85f }, glm::vec2{ 0.3675f, 0.1f }, glm::vec3{ 0.045f, 0.045f, 0.045f });
		auto learnButton = std::make_shared<ke::ui::Button>(glm::vec2{ -1.0f + 2.0f * 0.3775f, -0.85f }, glm::vec2{ 0.3675f, 0.1f }, glm::vec3{ 0.045f, 0.045f, 0.045f });
		auto docsButton = std::make_shared<ke::ui::Button>(glm::vec2{ -1.0f + 3.0f * 0.3775f, -0.85f }, glm::vec2{ 0.3675f, 0.1f }, glm::vec3{ 0.045f, 0.045f, 0.045f });
		tabs->addToChildren(overviewButton);
		tabs->addToChildren(changelogButton);
		tabs->addToChildren(learnButton);
		tabs->addToChildren(docsButton);

		window.addToInteractable(minimizeButton);
		window.addToInteractable(exitButton);

		while (!window.shouldClose())
		{
			window.processInput();

			if (!window.hasIconified())
			{
				renderer.beginRecording(window.getWindow(), window.hasResized());
				if (renderer.hasRecreatedSwapchain())
				{
					window.setResized(false);
					continue;
				}
				VkCommandBuffer commandBuffer = renderer.getCommandBuffer();
				// DRAW CALLS GO HERE
				topBar->Draw(commandBuffer);
				rightBar->Draw(commandBuffer);
				tabs->Draw(commandBuffer);

				renderer.endRecording();
				renderer.present(window.getWindow());

				renderer.advanceFrame();

			}

			renderer.destroyRedundantBuffers();
			window.pollEvents();
		}

		window.clearInteractable();
	}
	   
	renderer.cleanupRenderer();
}


