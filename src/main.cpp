#include "window.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	ke::Window window(800, 800, "Hello, World!");

	while (!window.shouldClose())
	{
		window.pollEvents();
	}
}