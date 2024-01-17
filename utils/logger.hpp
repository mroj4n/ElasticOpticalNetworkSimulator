#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

namespace Logger
{
enum class LogType
{
    INFO,
    DEBUG,
    WARNING,
    ERROR
};
class Logger
{
private:
    std::string logFilePath;
    std::ofstream logFile;
    bool printDebug = false;
    uint16_t logFileLineLimit = 10000;
    void clearLogFile()
    {
        logFile.open(logFilePath, std::ofstream::out | std::ofstream::trunc);
        logFile.close();
    }
    void deleteLineFromTopIfLimitIsSurpassed()
    {
        std::ifstream file(logFilePath);
        std::string line;
        uint16_t lineCount = 0;
        while (std::getline(file, line))
        {
            lineCount++;
        }
        file.close();
        if (lineCount > logFileLineLimit)
        {
            std::ifstream file(logFilePath);
            std::ofstream tempFile("temp.log");
            uint16_t lineCount = 0;
            while (std::getline(file, line))
            {
                if (lineCount > 1)
                {
                    tempFile << line << std::endl;
                }
                lineCount++;
            }
            file.close();
            tempFile.close();
            remove(logFilePath.c_str());
            rename("temp.log", logFilePath.c_str());
        }
    }
public:

    Logger()
    {
        logFilePath = "log.log";
        clearLogFile();
    }
    Logger(std::string logFilePath, bool printDebug = false)
    {
        this->printDebug = printDebug;
        this->logFilePath = logFilePath;
        clearLogFile();
    }
    std::string getFlag(LogType logType)
    {
        switch (logType)
        {
        case LogType::INFO:
            return "[INF]";
            break;
        case LogType::WARNING:
            return "[WRN]";
            break;
        case LogType::DEBUG:
            return "[DBG]";
            break;
        case LogType::ERROR:
            return "[ERR]";
            break;
        default:
            return "[UNKNOWN]";
            break;
        }
    }
    void info(std::string message)
    {
        logFile.open(logFilePath, std::ofstream::out | std::ofstream::app);
        logFile << getFlag(LogType::INFO) << " " << message << std::endl;
        logFile.close();
    }
    void debug(std::string message)
    {
        if(!printDebug){
            return;
        }
        logFile.open(logFilePath, std::ofstream::out | std::ofstream::app);
        logFile << getFlag(LogType::DEBUG) << " " << message << std::endl;
        logFile.close();
    }
    void warning(std::string message)
    {
        logFile.open(logFilePath, std::ofstream::out | std::ofstream::app);
        logFile << getFlag(LogType::WARNING) << " " << message << std::endl;
        logFile.close();
    }
    void error(std::string message)
    {
        logFile.open(logFilePath, std::ofstream::out | std::ofstream::app);
        logFile << getFlag(LogType::ERROR) << " " << message << std::endl;
        logFile.close();
    }
    ~Logger()
    {
    }
};
}
extern Logger::Logger logger;