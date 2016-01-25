#ifndef TRANSITION_TABLE_H
#define TRANSITION_TABLE_H

#include <vector>
#include <string>

namespace evaluator_internal
{

// State transition table record
class transition_table_record
{
public:

    std::vector<std::string> Terminals;
    int Jump;
    bool Accept;
    bool Stack;
    bool Return;
    bool Error;
    void set_values(const std::string & T, int J, bool A, bool S, bool R, bool E);
};

}

#endif // TRANSITION_TABLE_H

