#include "logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

ke::Logger::Logger(std::string n, spdlog::level::level_enum l)
	:mLoggerName(n)
{
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	mLogger = std::make_shared<spdlog::logger>(n, consoleSink);

	mLogger.get()->set_pattern("[%H:%M:%S] %^%n %l:%$ %v");
	mLogger.get()->set_level(l);
}

void ke::Logger::trace(std::string content)
{
	mLogger.get()->trace(content);
}

void ke::Logger::debug(std::string content)
{
	mLogger.get()->debug(content);
}

void ke::Logger::info(std::string content)
{
	mLogger.get()->info(content);
}

void ke::Logger::warn(std::string content)
{
	mLogger.get()->warn(content);
}

void ke::Logger::error(std::string content)
{
	mLogger.get()->error(content);
}

void ke::Logger::critical(std::string content)
{
	mLogger.get()->critical(content);
}


