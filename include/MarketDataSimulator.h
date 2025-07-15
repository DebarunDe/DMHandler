#pragma once
#include <string>
#include <functional>
#include <chrono>

class MarketDataSimulator {
private:
    std::string dataSource_; // filepath 
    bool isStatic_; // true if using static data from file, false if generating data dynamically
    bool runUniform_; // true if running uniformly, false if running with random bursts/slowdowns
    int msgPerSecond_; // number of messages per second to simulate
    int msgCount_; // total number of messages to simulate
public:
    MarketDataSimulator();
    //primarily used for static case from csv file
    explicit MarketDataSimulator(bool isStatic);
    //primary used for dynamic case
    MarketDataSimulator(bool runUniform, int msgPerSecond, int msgCount);

    ~MarketDataSimulator();

    const std::string& getDataSource()     const { return dataSource_;     };
    const bool         isStatic()          const { return isStatic_;       };
    const bool         isRunUniform()      const { return runUniform_;     };
    const int          getMsgPerSecond()   const { return msgPerSecond_;   };
    const int          getMsgCount()       const { return msgCount_;       };    

    void sleep(std::chrono::microseconds interval);
    void run(std::function<void(const std::string&)> callback);

};