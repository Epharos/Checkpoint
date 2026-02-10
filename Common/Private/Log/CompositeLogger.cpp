#include "CompositeLogger.hpp"

#include "../Assert.hpp"

namespace cp
{
	CompositeLogger::CompositeLogger() : ILogger(nullptr)
	{

	}

	void CompositeLogger::AddLogger(std::shared_ptr<ILogger> _logger)
	{
		std::lock_guard<std::mutex> lock(mutex);

		loggers.push_back(_logger);
	}

	void CompositeLogger::Log(std::string_view _message, LogLevel _logLevel)
	{
		std::lock_guard<std::mutex> lock(mutex);

		for (auto& logger : loggers)
		{
			logger->Log(_message, _logLevel);
		}
	}

	void CompositeLogger::Log(const Message& _message, LogLevel _logLevel)
	{
		std::lock_guard<std::mutex> lock(mutex);

		for (auto& logger : loggers)
		{
			logger->Log(_message, _logLevel);
		}
	}

	void CompositeLogger::Log(const LogEvent& _event)
	{
		std::lock_guard<std::mutex> lock(mutex);

		for (auto& logger : loggers)
		{
			logger->Log(_event);
		}
	}

	void CompositeLogger::Spacing(size_t _verticalSpaces)
	{
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& logger : loggers)
		{
			logger->Spacing(_verticalSpaces);
		}
	}
}