#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include "operator_types.h"
#include "global_properties.h"

#include "../../parser_json/src/packet_parser.h"

class BasicCondition;

class Sensor {
public:
    int id;
    std::string name;
    PacketParser *parser;
    std::map<std::string, Field> fieldsMap;

    // Variables for the timer functions
    int timeForUpdate;
    std::atomic<int> timerCounter; // Flag to indicate the remaining time in seconds
    std::mutex mtx;
    std::thread timerThread;

    // Contains the current values of various fields and a list of basic conditions associated with each field
    std::map<std::string, std::pair<FieldValue, std::vector<BasicCondition *>>> fields;

    // C-tor initializes the id member variable.
    Sensor(int id, string name, string jsonFilePath);

    // Updates the condition status according to the received field and returns the  list of the full conditions whose root is true
    void updateTrueRoots(std::string field, FieldValue value, FieldType type);

    void handleMessage(void *msg);

private:
    template <typename T>
    bool applyComparison(T a, T b, OperatorTypes op);

    // Function that running until the timeout is reached and activate the update.
    void delayedFunction();
    // Function that activate or reset the timer
    void startOrResetTimer();
    // Function that update the fields to be the default values
    void updateDefualtValues();
};

#endif  // _SENSOR_H_