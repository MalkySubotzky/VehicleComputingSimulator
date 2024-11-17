#ifndef __FULL_CONDITION_H__
#define __FULL_CONDITION_H__

#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <stack>
#include <unordered_map>

#include "sensor.h"
#include "or_operator.h"
#include "and_operator.h"
#include "basic_condition.h"
#include "root.h"
#include "condition_factory.h"
#include "hsm_support.h"

class Root;

class FullCondition {
private:
    // Recursively builds the condition tree from the condition string.
    Condition *buildNode(const string &condition, int &index,
                         map<int, int> bracketIndexes);

public:
    // Global map to keep track of existing conditions to avoid duplication
    static unordered_map<string, Condition *> s_existingConditions;
    // Static counter to assign unique IDs to each FullCondition instance
    static int s_counter;
    // Unique ID for the FullCondition instance
    int id;
    // Root node of the condition tree
    Root *root;
    // Vector of actions associated with this condition
    vector<pair<int, string>> actions;

    // Constructor: Initializes the FullCondition object, parses the condition string, and builds the condition tree.
    FullCondition(string condition, vector<pair<int, string>> &actions);

    void activateActions();
};

#endif  // _FULL_CONDITION_H_