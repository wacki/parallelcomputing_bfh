#pragma once

#include <string>
#include <iostream>
#include <fstream>


enum LogVerbosity {
    LV_Quiet = 0,
    LV_Minimal = 1,
    LV_Normal = 2,
    LV_Detailed = 3,
    LV_Diagnostic = 4
};

// log class to write to file and console
class Log
{
public:

    ~Log()
    {
        if (_ofs.is_open())
            _ofs.close();
    }

    static Log& singleton()
    {
        static Log log;
        return log;
    }

    static void add(LogVerbosity verb, const std::string& msg)
    {
        singleton().logMsg(verb, msg);
    }
    
    void open(const std::string& file)
    {
        _ofs.open(file);
    }
    
    void setVerbosity(LogVerbosity verb) { _verbosity = verb; }
    LogVerbosity getVerbosity() const { return _verbosity; }
    
    void logMsg(LogVerbosity verb, const std::string& msg)
    {
        if (_verbosity < verb)
            return;

        std::cout << msg;
        std::cout.flush();
        _ofs << msg;
        _ofs.flush();
    }

private:
    // prevent instantiation
    Log()
        : _verbosity(LV_Diagnostic)
    {}

    LogVerbosity    _verbosity;
    std::ofstream   _ofs;

};