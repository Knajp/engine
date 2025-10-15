#pragma once
#include <vector>
#include <fstream>
#include <iostream>

namespace ke
{
	namespace util
	{
		std::vector<char> readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);;
			if (!file.is_open())
				std::cerr << "Utility error: failed to open file " << filename << "\n";

			size_t filesize = (size_t)file.tellg();
			std::vector<char> buffer(filesize);

			file.seekg(0);
			file.read(buffer.data(), filesize);

			return buffer;
		}
	}
}