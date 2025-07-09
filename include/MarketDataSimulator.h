#pragma once
#include <string>
#include <functional>
#include <chrono>

/* want something that is capable of having a static repeatable handler (from csv file) 
*   and something that can create data on its own in-memory
*   and something that can be used to simulate market data for testing purposes.
*/


class MarketDataSimulator {
private:
    std::string dataSource_; // filepath 
    bool isStatic_; // true if using static data from file, false if generating data dynamically
    int msgPerSecond_; // number of messages per second to simulate
    bool runUniform_; // true if running uniformly, false if running with random bursts/slowdowns
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
    int         getMsgPerSecond()   const { return msgPerSecond_;   };
    bool        isRunUniform()      const { return runUniform_;     };
    int         getMsgCount()       const { return msgCount_;       };    

    void sleep();
    void run(std::function<void(const std::string&)> callback);

};