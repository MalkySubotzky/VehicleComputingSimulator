#include <iostream>
#include <vector>
#include "sensor.h"
#include "input.h"
#include "full_condition.h"
#include "global_properties.h"

using namespace std;

int main()
{
    GlobalProperties &instanceGP = GlobalProperties::getInstance();
    // Build the conditions from the bson file
    Input::s_buildConditions();

    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::INFO,
        "Initialized successfully, Starting Communication...");
    // Starting communication with the server
    instanceGP.comm->startConnection();

    // Running in a time loop to receive messages and handle them
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    GlobalProperties::controlLogger.cleanUp();

    return 0;
}