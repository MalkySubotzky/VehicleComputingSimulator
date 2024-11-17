#ifndef __GLOBAL_PROPERTIES_H__
#define __GLOBAL_PROPERTIES_H__

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include <nlohmann/json.hpp>
#include "input.h"
#include "full_condition.h"
#include "sensor.h"
#include "hsm_support.h"

#include "../../communication/include/communication.h"
#include "../../logger/logger.h"

using json = nlohmann::json;

class FullCondition;
// Forward declaration instead of #include
class Sensor;

constexpr int BITS_IN_BYTE = 8;

// Singleton class managing global properties
class GlobalProperties {
private:
    // Single instance of the class
    static unique_ptr<GlobalProperties> instance;
    // Private constructor
    GlobalProperties();

public:
    // Gets the singleton instance
    static GlobalProperties &getInstance();
    // Resets the singleton instance
    static void resetInstance();

    // Map of sensors by ID
    std::unordered_map<int, Sensor *> sensors;
    // Map of conditions by ID
    std::unordered_map<int, FullCondition *> conditions;
    // Set of true condition IDs
    std::unordered_set<int> trueConditions;

    // Communication
    uint32_t srcID = 1;
    // Creating the communication object with the callback function to process the data
    Communication *comm;
    
    static logger controlLogger;
};

#endif  // _GLOBAL_PROPERTIES_H_