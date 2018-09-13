#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <Shlwapi.h>

//#define SAVE_LOG_FILE

class GlobalLogger
{
    enum STATUS
    {
        DEBUGINFO, MESSAGE, WARNING, ERR, SECTION
    };
public:
    GlobalLogger(const SYSTEMTIME &_dateTime, const std::string &_resultFolder)
    {
        m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        m_status = MESSAGE;

        // get date and time
        char dateTime[256] = { 0 };
        sprintf(dateTime, "-%02d_%02d_%02d_%02d_%02d",
                _dateTime.wMonth, _dateTime.wDay, _dateTime.wHour, _dateTime.wMinute, _dateTime.wSecond);

        // get the name of the program
        TCHAR buffer[512] = { 0 };
        TCHAR * out;
        DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

        // Get the fully-qualified path of the executable
        if (GetModuleFileName(NULL, buffer, bufSize) == bufSize)
        {
            throw std::runtime_error("The path of the executable is too long!!");
        }
        out = PathFindFileName(buffer);
        *(PathFindExtension(out)) = 0;

        // obtain the name of the logger file
        m_filename = _resultFolder + "./" + std::string(out) + std::string(dateTime) + ".log";
        m_logBuffer.reserve(bufSize);
#ifdef SAVE_LOG_FILE
        std::ofstream of(m_filename);
        if (!of.is_open()) throw std::runtime_error("Failed to open/create log file!!!");
        of.clear();
        of.close();
#endif //SAVE_LOG_FILE
    }

    ~GlobalLogger()
    {
        saveBufToFile();
    }

    GlobalLogger& debugInfo(int _line, const char* _file) // WHITE
    {
        m_status = DEBUGINFO;
        printTime();
        SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
        std::cout << "Debug info at line " << _line << " of file " << _file << std::endl;


        std::stringstream ss;
        ss << "Debug info at line " << _line << " of file " << _file << std::endl;
        m_logBuffer += ss.str();
        if (m_logBuffer.size() > 0.9*m_bufSize) saveBufToFile();
        
        return *this;
    }

    GlobalLogger& message() // GREEN
    {
        m_status = MESSAGE;
        printTime();
        return *this;
    }

    GlobalLogger& warning()  // YELLOW
    {
        m_status = WARNING;
        printTime();
        return *this;
    }

    GlobalLogger& error()  // RED
    {
        m_status = ERR;
        printTime();
        return *this;
    }

    GlobalLogger& sectionTitle()   // PURPLE
    {
        m_status = SECTION;
        printTime();
        return *this;
    }

    template<typename T>
    GlobalLogger& operator << (T &txt)
    {
        switch (m_status)
        {
        case MESSAGE:
            SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_GREEN);
            break;
        case WARNING:
            SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        case ERR:
            SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        case SECTION:
            SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_BLUE | FOREGROUND_RED);
            break;
        case DEBUGINFO:
            SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            break;
        default:
            SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            break;
        }

        std::cout << txt;

        std::stringstream ss;
        ss << txt;
        m_logBuffer += ss.str();
        if (m_logBuffer.size() > 0.9*m_bufSize) saveBufToFile();

        // restore the console style
        SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
        return *this;
    }

    GlobalLogger& GlobalLogger::operator << (std::ostream& (*endline)(std::ostream&))
    {
        std::cout << endline;

        std::stringstream ss;
        ss << endline;
        m_logBuffer += ss.str();
        if (m_logBuffer.size() > 0.9*m_bufSize) saveBufToFile();

        SetConsoleTextAttribute(m_consoleHandle, 7);
        return *this;
    }

    // print time infomation
    void printTime()
    {
        SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_GREEN | FOREGROUND_BLUE);
        GetLocalTime(&m_sys);
        printf("[%02d/%02d %02d:%02d:%02d.%03d] ", m_sys.wMonth, m_sys.wDay, m_sys.wHour, m_sys.wMinute, m_sys.wSecond, m_sys.wMilliseconds);
    }

    void saveBufToFile()
    {
#ifdef SAVE_LOG_FILE
        std::ofstream of(m_filename, std::ios::app);
        of << m_logBuffer;
        of.close();
        m_logBuffer.clear();
#endif //SAVE_LOG_FILE
    }

private:
    SYSTEMTIME m_sys;
    STATUS m_status;
    HANDLE m_consoleHandle;
    std::string m_filename;
    int m_bufSize = 1000000;
    std::string m_logBuffer;
};

extern GlobalLogger logger;
#define logErr logger.error()
#define logMsg logger.message()
#define logWrn logger.warning()
#define logSec logger.sectionTitle()
#define logDbg logger.debugInfo()
#define logTime logger.printTime()
