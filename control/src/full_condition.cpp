#include "full_condition.h"
using namespace std;

// Global pointer to the current sensor on which the condition is conditional
Sensor *currentSensor;

// Global map to keep track of existing conditions to avoid duplication
unordered_map<string, Condition *> FullCondition::s_existingConditions = {};

// Initialize a static variable to assign unique IDs to each FullCondition instance
int FullCondition::s_counter = 0;

// Handling sensor reference
void defineCurrentSensor(const string &condition, int &index)
{
    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    int closeBracket = find(condition.begin() + index, condition.end(), ']') -
                       condition.begin();
    string numStr = condition.substr(index + 1, closeBracket - index - 1);
    int id = stoi(numStr);
    index = closeBracket + 1;
    currentSensor = instanceGP.sensors[id];

    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::DEBUG,
        "The current sensor id is: " + to_string(currentSensor->id));
}

// Recursively builds the condition tree from the condition string.
Condition *FullCondition::buildNode(const string &condition, int &index,
                                    map<int, int> bracketIndexes)
{
    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::DEBUG,
        "Entering buildNode function, condition[index] = " + condition[index]);
    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    if (condition.empty())
        GlobalProperties::controlLogger.logMessage(logger::LogLevel::ERROR,
                                                   "Condition string is empty");

    // Handling sensor reference
    if (condition[index] == '[')
        defineCurrentSensor(condition, index);

    int openBracketIndex =
        find(condition.begin() + index, condition.end(), '(') -
        condition.begin();

    // Generates a key for the condition with the current sensor's ID (if exists)
    string key =
        (currentSensor ? to_string(currentSensor->id) : "-") +
        condition.substr(index, bracketIndexes[openBracketIndex] - index + 1);

    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::DEBUG, "Generated condition key: " + key);

    // Check if the key already exists in the existingConditions map
    if (s_existingConditions.find(key) != s_existingConditions.end()) {
        instanceGP.controlLogger.logMessage(
            logger::LogLevel::DEBUG, "Condition key already exists: " + key);

        index = bracketIndexes[openBracketIndex] + 1;
        if (condition[index] == ',')
            index++;
        return s_existingConditions.find(key)->second;
    }

    // | , & , ( = , < , > , >= , <= , != )
    // Creating a new node
    OperatorTypes operatorType = convertStringToOperatorTypes(
        condition.substr(index, openBracketIndex - index));

    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::DEBUG,
        "Operator type: " + std::to_string(operatorType));

    Condition *conditionPtr = createCondition(operatorType);

    s_existingConditions[key] = conditionPtr;

    if (OperatorNode *operatorNode =
            dynamic_cast<OperatorNode *>(conditionPtr)) {
        index += 2;
        int count = 0;

        // Going over the internal conditions and creating children
        while (condition[index] != ')') {
            // Handling sensor reference
            if (condition[index] == '[')
                defineCurrentSensor(condition, index);

            // Handling same operator
            string temp = condition.substr(index, 1);
            while (convertStringToOperatorTypes(temp) == operatorType) {
                count++;
                index += 2;
                temp = condition.substr(index, 1);
            }

            operatorNode->conditions.push_back(
                buildNode(condition, index, bracketIndexes));
            operatorNode->conditions[operatorNode->conditions.size() - 1]
                ->parents.push_back(operatorNode);

            while (condition[index] == ')' && count) {
                count--;
                index++;
            }
            if (condition[index] == ',')
                index++;
        }
    }
    else if (BasicCondition *basicCondition =
                 dynamic_cast<BasicCondition *>(conditionPtr)) {
        // Fill the fields in BasicCondition
        int commaIndex = find(condition.begin() + index, condition.end(), ',') -
                         condition.begin();
        string name = condition.substr(openBracketIndex + 1,
                                       commaIndex - openBracketIndex - 1);

        GlobalProperties::controlLogger.logMessage(logger::LogLevel::DEBUG,
                                                   "Field name: " + name);

        int closeBracket = bracketIndexes[openBracketIndex];

        // Extract the substring
        string valueStr =
            condition.substr(commaIndex + 1, closeBracket - commaIndex - 1);

        // Convert the string to a void pointer
        basicCondition->value = valueStr;
        basicCondition->setValue(valueStr,
                                 currentSensor->parser->getFieldType(
                                     currentSensor->fieldsMap[name].type));

        // Add the sensor reference to this leaf
        currentSensor->fields[name].second.push_back(basicCondition);
    }

    index = bracketIndexes[openBracketIndex] + 1;
    if (condition[index] == ',')
        index++;

    return conditionPtr;
}

// Maps the positions of opening bracket indexes to their corresponding closing bracket indexes
map<int, int> findBrackets(string condition)
{
    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::DEBUG, "Generate a map with the brackets indexes");
    map<int, int> mapIndexes;
    stack<int> stackIndexes;
    // Scans the input string for brackets and uses a stack to keep track of their positions
    for (int i = 0; i < condition.size(); i++) {
        if (condition[i] == '(') {
            stackIndexes.push(i);
        }
        else if (condition[i] == ')') {
            mapIndexes[stackIndexes.top()] = i;
            stackIndexes.pop();
        }
    }

    // Returning a map where each opening bracket index is associated with its matching closing bracket index
    return mapIndexes;
}

// Constructor: Builds the condition tree.
FullCondition::FullCondition(string condition,
                             vector<pair<int, string>> &actions)
    : actions(actions)
{
    // Sets a unique ID for the condition and creates the root of the condition tree
    id = FullCondition::s_counter++;

    // Initializes the condition tree based on the provided condition string and actions map
    map<int, int> bracketsIndexes = findBrackets(condition);
    int index = 0;
    Condition *firstCondition =
        this->buildNode(condition, index, bracketsIndexes);
    root = new Root(this->id, firstCondition);

    GlobalProperties::controlLogger.logMessage(
        logger::LogLevel::DEBUG, "The tree created successfully ");

    firstCondition->parents.push_back(root);
    currentSensor = nullptr;
}

// Fuction that activates all actions in the vector
void FullCondition::activateActions()
{
    // Activates all actions associated with the `FullCondition`
    GlobalProperties &instanceGP = GlobalProperties::getInstance();
    for (pair<int, string> action : this->actions) {
        // Sending the message
        const char *message = action.second.c_str();
        size_t dataSize = strlen(message) + 1;
        uint32_t destID = action.first;
        if (instanceGP.sensors[destID]->isUsingHSM) {
            // Get the length of the encrypted data
            size_t encryptedLength =
                hsm::getEncryptedLen(instanceGP.srcID, dataSize);
            uint8_t encryptedData[encryptedLength];

            if (hsm::encryptData((const void *)message, dataSize, encryptedData,
                                 encryptedLength, instanceGP.srcID, destID)) {
                instanceGP.controlLogger.logMessage(
                    logger::LogLevel::INFO,
                    "The message encrypted successfully");
                instanceGP.comm->sendMessage(encryptedData, encryptedLength,
                                             destID, instanceGP.srcID, false);
            }
            else {
                instanceGP.controlLogger.logMessage(
                    logger::LogLevel::ERROR, "The message encryption failed");
                instanceGP.comm->sendMessage((void *)message, dataSize, destID,
                                             instanceGP.srcID, false);
            }
        }
        else
        instanceGP.comm->sendMessage((void *)message, dataSize, destID,
                                     instanceGP.srcID, false);
    }
}