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
    MarketDataSimulator(bool isStatic);
    //primary used for dynamic case
    MarketDataSimulator(int msgPerSecond, bool runUniform, int msgCount);

    ~MarketDataSimulator();

    std::string getDataSource()     const { return dataSource_;     };
    bool        isStatic()          const { return isStatic_;       };
    bool        isRunUniform()      const { return runUniform_;     };
    int         getMsgPerSecond()   const { return msgPerSecond_;   };
    int         getMsgCount()       const { return msgCount_;       };    

    void sleep(std::chrono::microseconds interval);
    void run(std::function<void(const std::string&)> callback);

};