#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <mutex>

enum class LogLevel {
    info,
    warning,
    error
};

class Logger {
public:
    Logger(const std::string &filename);
    ~Logger();

    void log(LogLevel level, const std::string &message);
    void setLogLevel(LogLevel level);

    static Logger& getInstance() {
        static Logger instance("app.log");
        return instance;
    }

private:
    std::ofstream logFile;
    LogLevel logLevel;
    std::mutex logMutex;

    std::string getTimestamp();
    std::string logLevelToString(LogLevel level);

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

inline Logger::Logger(const std::string &filename) : logLevel(LogLevel::info) {
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        exit(1);
    }
}

inline Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

inline void Logger::log(LogLevel level, const std::string &message) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (level >= logLevel) {
        logFile << getTimestamp() << " [" << logLevelToString(level) << "] " << message << std::endl;
        std::cout << getTimestamp() << " [" << logLevelToString(level) << "] " << message << std::endl;
    }
}

inline void Logger::setLogLevel(LogLevel level) {
    logLevel = level;
}

inline std::string Logger::getTimestamp() {
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

inline std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::info:
            return "info";
        case LogLevel::warning:
            return "warning";
        case LogLevel::error:
            return "error";
        default:
            return "unknown";
    }
}

// Global logger instance
#define LOG_INFO(message) Logger::getInstance().log(LogLevel::info, message)
#define LOG_WARNING(message) Logger::getInstance().log(LogLevel::warning, message)
#define LOG_ERROR(message) Logger::getInstance().log(LogLevel::error, message)

#endif // LOGGER_H