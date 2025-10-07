#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
namespace ke {
	class Logger
	{
	public:
		Logger(std::string n, spdlog::level::level_enum l);

		void trace(std::string content);
		void debug(std::string content);
		void info(std::string content);
		void warn(std::string content);
		void error(std::string content);
		void critical(std::string content);
	private:
		std::string mLoggerName;

		std::shared_ptr<spdlog::logger> mLogger;
	};
}