#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
namespace ke {
	class Logger
	{
	public:
		Logger(std::string n, spdlog::level::level_enum l);

		void trace(std::string content) const;
		void debug(std::string content) const;
		void info(std::string content) const;
		void warn(std::string content) const;
		void error(std::string content) const;
		void critical(std::string content) const;
	private:
		std::string mLoggerName;

		std::shared_ptr<spdlog::logger> mLogger;
	};
}